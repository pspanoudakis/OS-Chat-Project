/*
 * File: reader.c
 * Pavlos Spanoudakis (sdi18000184)
 * 
 * Child process in P1 and P2, used for printing to stdout.
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
#include <errno.h>

#include "utils.h"

// Global declerations (in order to be visible from quit())
int semid, shmid;
char* msg, *shmem;
struct sembuf *ops;

/* Used for Terminating properly (freeing heap memory, dettaching pointers 
& deleting semaphores and shared memory segments) */
void quit(int signum)
{
    struct shmid_ds temp;

    free(ops);

    if (shmdt(shmem) == -1) { exit_failure("Could not attach pointer to shared memory.\n"); }    
    
    if (shmctl(shmid, IPC_STAT, &temp) != -1)               // Getting Shared Memory info
    {
        if (temp.shm_nattch == 0)
        // Delete only if there are no attached pointers to shared memory left
        {
            if ( shmctl(shmid, IPC_RMID, 0) == -1 ){ exit_failure("Deleting SHM failed\n"); }
        }
    }
    
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
    pid_t child_id;
    union semun args;

    if (argc < 3)
    {
        perror("Insufficient arguments\n");
        exit(EXIT_FAILURE);
    }
    // Getting semaphore and shared memory keys
    key_t shm_source_key = (key_t)atoi(argv[1]);
    key_t sem_source_key = (key_t)atoi(argv[2]);

    // To terminate smoothly if a SIGTERM is sent
    signal(SIGTERM, quit);

    // Using 2 semaphores for shared memory handling
    semid = semget(sem_source_key, 2, IPC_CREAT|PERMS);
    if (semid == -1)
    {
        perror("Did not get semaphore\n");
        exit(EXIT_FAILURE);
    }
    // Initiallize the first to 0
    sem_init(semid, 0, 0);
    // And the second to 1
    sem_init(semid, 1, 1);

    // Aquiring the shared memory segment for reading
    shmid = shmget(shm_source_key, SHMSIZE, IPC_CREAT|PERMS);
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

    msg = malloc(1);                                                    // messages will be stored here before being printed
    if (msg == NULL) { malloc_error_exit(); }
    // Initiallizing msg with a null character so strcmp can be safely called
    msg[0] = '\0';

    // Loop until a termination message has been read
    while (strcmp(msg, EXIT_MESSAGE) != 0)
    {
        free(msg);

        // If the operation is not succesful, a termination message has probably been sent from the opposite direction
        // which caused the semaphored to be deleted, so terminate
        if (sem_down(semid, ops, 0) == -1) { quit(SIGTERM); }   

        printf("Just Read: %s\n", shmem);
        msg = malloc(strlen(shmem) + 1);
        if (msg == NULL) { malloc_error_exit(); }

        // Reading from source shared memory
        strcpy(msg, shmem);
        
        sem_up(semid, ops, 1);
    }

    free(msg);
    quit(SIGTERM);
}