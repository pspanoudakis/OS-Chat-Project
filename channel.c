/*
 * File: channel.c
 * Pavlos Spanoudakis (sdi18000184)
 * 
 * Child process of CHAN, used for message transmissions and modifications.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <openssl/md5.h>
#include <errno.h>

#include "utils.h"

// Global declerations (in order to be visible from quit())
char *msg, *shmsrc, *shmdest;
struct sembuf *ops;
int semsrcid, shmsrcid, semdestid, shmdestid;

/* Used for Terminating properly (freeing heap memory, dettaching pointers 
& deleting semaphores and shared memory segments) */
void quit(int signum)
{
    // Freeing allocated memory
    free(ops);

    struct shmid_ds temp;                                               // Used to get Shared Memory info

    // Detaching pointer to shared memory for reading
    if (shmdt(shmsrc) == -1) { exit_failure("Could not attach pointer to shared memory.\n"); }    
    
    if (shmctl(shmsrcid, IPC_STAT, &temp) != -1)                        // Getting Shared Memory info
    {
        if (temp.shm_nattch == 0)
        // Delete only if there are no attached pointers to shared memory left
        {
            if ( shmctl(shmsrcid, IPC_RMID, 0) == -1 ){ exit_failure("Deleting SHM failed\n"); }
        }
    }
    
    // Detaching pointer to shared memory for writting
    if (shmdt(shmdest) == -1) { exit_failure("Could not detach pointer to shared memory.\n"); }    
    
    if (shmctl(shmdestid, IPC_STAT, &temp) != -1)                       // Getting Shared Memory info
    {
        if (temp.shm_nattch == 0)
        // Delete only if there are no attached pointers to shared memory left
        {
            if ( shmctl(shmdestid, IPC_RMID, 0) == -1 ){ printf("Deleting SHM failed\n"); }            
        }
    }
    
    // Deleting semaphore for destination shared memory (if it has not been deleted already)
    if ( (semctl(semdestid, 0, IPC_RMID, 0) == -1) )
    {
        // An EINVAL error is expected if the semaphore has been deleted
        if(errno != EINVAL){ printf("Deleting semaphore failed\n"); }  
    }
    
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
    union semun args;

    // To terminate smoothly if a SIGTERM is sent
    signal(SIGTERM, quit);    

    // Getting the keys for semaphores and shared memory
    key_t sem_source_key = (key_t)atoi(argv[1]);
    key_t sem_dest_key = (key_t)atoi(argv[2]);
    key_t shm_source_key = (key_t)atoi(argv[3]);
    key_t shm_dest_key = (key_t)atoi(argv[4]);
    int chance = atoi(argv[5]);

    semdestid = semget(sem_dest_key, 2, IPC_CREAT|PERMS);           // 2 semaphores to handle shared memory for writing
    semsrcid = semget(sem_source_key, 2, IPC_CREAT|PERMS);          // 2 semaphores to handle shared memory for reading
    if (shmsrc == (char*)-1 || shmdest == (char*)-1)
    {
        perror("Failed to attach\n");
        exit(EXIT_FAILURE);
    }

    // Initiallizing the semaphores
    sem_init(semdestid, 0, 0);
    sem_init(semdestid, 1, 1);
    sem_init(semsrcid, 0, 0);
    sem_init(semsrcid, 1, 1);

    shmdestid = shmget(shm_dest_key, SHMSIZE, IPC_CREAT|PERMS);     // Getting shared memory for writing
    shmsrcid = shmget(shm_source_key, SHMSIZE, IPC_CREAT|PERMS);    // Getting shared memory for reading
    if (shmdestid == -1)
    {
        perror("Did not get shared memory\n");
        exit(EXIT_FAILURE);
    }

    // Attaching pointers to shared memory segments
    shmdest = (char*)shmat(shmdestid, (char*)0, 0);
    shmsrc = (char*)shmat(shmsrcid, (char*)0, 0);
    if (shmdest == (char*)-1)
    {
        perror("Failed to attach\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));                                              // rand() will be used for modifying messages

    ops = malloc(sizeof(struct sembuf));                            // used for semaphore operations
    if (ops == NULL) { malloc_error_exit(); }

    msg = malloc(1);                                                // messages will be stored here before being forwarded
    if (msg == NULL) { malloc_error_exit(); }
    // Initiallizing msg with a null character so strcmp can be safely called
    msg[0] = '\0';

    // Loop until a termination message has been read
    while ( memcmp(msg, EXIT_MESSAGE, strlen(EXIT_MESSAGE) + 1) != 0 )
    {
        free(msg);
        // Reading from source shared memory
        // Quiting if the operation fails (most likely a termination message has been sent from the opposite direction)
        if ( sem_down(semsrcid, ops, 0) == -1) { quit(SIGTERM); }

        msg = malloc(strlen(shmsrc) + 1 + MD5_DIGEST_LENGTH);
        if (msg == NULL) { malloc_error_exit(); }
        memcpy(msg, shmsrc, strlen(shmsrc) + 1 + MD5_DIGEST_LENGTH);

        sem_up(semsrcid, ops, 1);
        if ( (memcmp(msg, EXIT_MESSAGE, strlen(EXIT_MESSAGE) + 1) != 0) 
            && (memcmp(msg, RESEND_MESSAGE, strlen(RESEND_MESSAGE) + 1) != 0) )
        {
            // Writting to destination shared memory
            // The message is not "special" so it can be modified
            sem_down(semdestid, ops, 1);
            memcpy(shmdest, msg, strlen(msg) + 1 + MD5_DIGEST_LENGTH);
            if ((rand() % 100) < chance)
            {
                // Modifying message
                write(STDOUT_FILENO, "Noise Added\n", strlen("Noise Added\n"));
                add_noise(shmdest);
            }
            sem_up(semdestid, ops, 0);
        }
        else
        {
            // An exit or resend message will not be modified
            sem_down(semdestid, ops, 1);
            memcpy(shmdest, msg, strlen(msg) + 1);
            sem_up(semdestid, ops, 0);
        }        
    }

    free(msg);
    quit(SIGTERM);
}