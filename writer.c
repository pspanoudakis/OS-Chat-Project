/*
 * File: writer.c
 * Pavlos Spanoudakis (sdi18000184)
 * 
 * Child process in P1 and P2, used for reading from stdin.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <openssl/md5.h>
#include <errno.h>

#include "utils.h"

// Global declerations (in order to be visible from the handler)
struct sembuf *ops;
int semid, shmid;
char *msg, *shmem;

/* Used for Terminating properly (freeing heap memory, dettaching pointers 
& deleting semaphores and shared memory segments) */
void sigterm_handler(int signum)
{
    // Freeing allocated memory
    free(msg);
    free(ops);

    struct shmid_ds temp;                                   // Used to get Shared Memory info

    // Detaching pointer to shared memory
    if (shmdt(shmem) == -1) { exit_failure("Could not attach pointer to shared memory.\n"); }    
    
    if (shmctl(shmid, IPC_STAT, &temp) != -1)               // Getting Shared Memory info
    {
        if (temp.shm_nattch == 0)
        // Delete only if there are no attached pointers to shared memory left
        {
            if ( shmctl(shmid, IPC_RMID, 0) == -1 ){ printf("Deleting SHM failed\n"); }
        }
    }

    // Deleting semaphore (if it has not been deleted already)
    if ( (semctl(semid, 0, IPC_RMID, 0) == -1) )
    {
        // An EINVAL error is expected if the semaphore has been deleted
        if(errno != EINVAL) { printf("Deleting semaphore failed\n"); }
    }
    
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
    union semun args;

    if (argc < 3)
    {
        perror("Insufficient arguments\n");
        exit(EXIT_FAILURE);
    }
    // Getting semaphore and shared memory keys
    key_t shm_dest_key = (key_t)atoi(argv[1]);
    key_t sem_dest_key = (key_t)atoi(argv[2]);

    // Parent process may send a SIGTERM, and we want sigterm_handler to handle it properly.
    signal(SIGTERM, sigterm_handler);

    // Using 2 semaphores for shared memory handling
    semid = semget(sem_dest_key, 2, IPC_CREAT|PERMS);
    if (semid == -1)
    {
        perror("Did not get semaphore\n");
        exit(EXIT_FAILURE);
    }
    // Initiallize the first to 0
    sem_init(semid, 0, 0);
    // And the second to 1
    sem_init(semid, 1, 1);

    // Aquiring the shared memory segment for writing
    shmid = shmget(shm_dest_key, SHMSIZE, IPC_CREAT|PERMS);
    if (shmid == -1)
    {
        perror("Did not get shared memory\n");
        exit(EXIT_FAILURE);
    }
    // Attaching a pointer to it
    shmem = (char*)shmat(shmid, (char*)0, 0);
    if (shmem == (char*)-1)
    {
        perror("Failed to attach\n");
        exit(EXIT_FAILURE);
    }

    // Make stdout unbuffered
    setbuf(stdout, NULL);

    ops = malloc(sizeof(struct sembuf));                                // used for semaphore operations
    if (ops == NULL) { malloc_error_exit(); }

    msg = malloc(1);                                                    // messages will be stored here before being forwarded
    if (msg == NULL) { malloc_error_exit(); }
    // Initiallizing msg with a null character so strcmp can be safely called
    msg[0] = '\0';

    // Loop until a termination message has been read
    while ( strcmp(msg, EXIT_MESSAGE) != 0 )
    {
        free(msg);
        // Get input message from stdin
        get_line(&msg);

        // Checking if the message does not exceed the maximum size
        if (strlen(msg) > (SHMSIZE - MD5_DIGEST_LENGTH - 1))
        {
            printf("Message must be up to %d characters.\n", (SHMSIZE - MD5_DIGEST_LENGTH - 1));
            free(msg);
            msg = malloc(1);
            if (msg == NULL) { malloc_error_exit(); }
            msg[0] = '\0';
            continue;
        }
        // Quiting if the operation fails (most likely a termination message has been sent from the opposite direction)
        if (sem_down(semid, ops, 1) == -1){ break; }

        printf("Writting: %s\n", msg);
        // Writting to destination shared memory
        strcpy(shmem, msg);

        sem_up(semid, ops, 0);
    }
    sigterm_handler(SIGTERM);
}