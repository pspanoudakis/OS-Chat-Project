/*
 * File: decrypt.c
 * Pavlos Spanoudakis (sdi18000184)
 * 
 * Child process in ENC1 and ENC2, used for decoding messages.
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

// Global declerations (in order to be visible from quit())
char *msg, *shmsrc, *shmdest, *resendshm, *sendbackshm;
struct sembuf *ops;
int semsrcid, shmsrcid, semdestid, shmdestid, resendsemid, resendshmid, sendbacksemid, sendbackshmid;

/* Used for Terminating properly (freeing heap memory, dettaching pointers 
& deleting semaphores and shared memory segments) */
void quit(int signum)
{
    struct shmid_ds temp;                                       // Used to get Shared Memory info

    // Freeing allocated memory
    free(ops);

    // Detaching pointer to shared memory for reading
    if (shmdt(shmsrc) == -1) { exit_failure("Could not detach pointer to shared memory.\n"); }    
    
    if (shmctl(shmsrcid, IPC_STAT, &temp) != -1)                // Getting Shared Memory info
    {
        if (temp.shm_nattch == 0)
        // Delete only if there are no attached pointers to shared memory left
        {
            if ( shmctl(shmsrcid, IPC_RMID, 0) == -1 ){ printf("Deleting SHM failed\n"); }
        }
    }
    
    // Detaching pointer to shared memory for reading
    if (shmdt(shmdest) == -1) { exit_failure("Could not detach pointer to shared memory.\n"); }    
    
    if (shmctl(shmdestid, IPC_STAT, &temp) != -1)               // Getting Shared Memory info
    {
        if (temp.shm_nattch == 0)
        // Delete only if there are no attached pointers to shared memory left
        {
            if ( shmctl(shmdestid, IPC_RMID, 0) == -1 ){ printf("Deleting SHM failed\n"); }
        }
    }
    
    if ( (semctl(semdestid, 0, IPC_RMID, 0) == -1) )
    { 
        if(errno != EINVAL) { printf("Decrypt: Deleting semaphore failed\n"); }  
    }
    
    // Detaching pointer to shared memory for sending retransmission requests
    if (shmdt(resendshm) == -1) { exit_failure("Could not detach pointer to shared memory.\n"); }   
    
    if (shmctl(resendshmid, IPC_STAT, &temp) != -1)             // Getting Shared Memory info
    {
        if (temp.shm_nattch == 0)
        // Delete only if there are no attached pointers to shared memory left
        {
            if ( shmctl(resendshmid, IPC_RMID, 0) == -1 ){ exit_failure("Deleting SHM failed\n"); }
        }
    }
    
    // Detaching pointer to shared memory for handling retransmission requests
    if (shmdt(sendbackshm) == -1) { exit_failure("Could not detach pointer to shared memory.\n"); }  
    
    if (shmctl(sendbackshmid, IPC_STAT, &temp) != -1)           // Getting Shared Memory info
    {
        if (temp.shm_nattch == 0)
        // Delete only if there are no attached pointers to shared memory left
        {
            if ( shmctl(sendbackshmid, IPC_RMID, 0) == -1 ){ exit_failure("Deleting SHM failed\n"); }
        }
    }
    
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
    union semun args;

    // To terminate smoothly if a SIGTERM is sent
    signal(SIGTERM, quit);

    if (argc < 9)
    {
        perror("Insufficient arguments\n");
        exit(EXIT_FAILURE);
    }

    // Getting semaphore and shared memory keys
    key_t sem_source_key = (key_t)atoi(argv[1]);
    key_t sem_dest_key = (key_t)atoi(argv[2]);
    key_t shm_source_key = (key_t)atoi(argv[3]);
    key_t shm_dest_key = (key_t)atoi(argv[4]);
    key_t resend_sem_key = (key_t)atoi(argv[5]);
    key_t resend_shm_key = (key_t)atoi(argv[6]);
    key_t sendback_sem_key = (key_t)atoi(argv[7]);
    key_t sendback_shm_key = (key_t)atoi(argv[8]);

    semdestid = semget(sem_dest_key, 2, IPC_CREAT|PERMS);               // 2 semaphores to handle shared memory for writing
    semsrcid = semget(sem_source_key, 2, IPC_CREAT|PERMS);              // to handle shared memory for reading
    resendsemid = semget(resend_sem_key, 2, IPC_CREAT|PERMS);           // to handle shared memory for sending retransmission requests
    sendbacksemid = semget(sendback_sem_key, 2, IPC_CREAT|PERMS);       // to handle shared memory for informing `encrypt` about a retransmission request
    if (semdestid == -1 || semsrcid == -1 || resendsemid == -1 || sendbacksemid == -1)
    {
        perror("Failed to get semaphore\n");
        exit(EXIT_FAILURE);
    }
    // Note that the semaphores should have been created and initiallized before this process starts
    // (parent1, parent2 and channel_parent *must* have been called first)

    shmdestid = shmget(shm_dest_key, SHMSIZE, IPC_CREAT|PERMS);         // shared memory for writing
    shmsrcid = shmget(shm_source_key, SHMSIZE, IPC_CREAT|PERMS);        // shared memory for reading
    resendshmid = shmget(resend_shm_key, SHMSIZE, IPC_CREAT|PERMS);     // shared memory for sending retransmission requests
    sendbackshmid = shmget(sendback_shm_key, SHMSIZE, IPC_CREAT|PERMS); // shared memory for informing `encrypt` about a retransmission request
    if (shmdestid == -1 || shmsrcid == -1 || resendshmid == -1 || sendbacksemid == -1)
    {
        perror("Did not get shared memory\n");
        exit(EXIT_FAILURE);
    }

    // Attaching pointers to every shared memory segment
    shmdest = (char*)shmat(shmdestid, (char*)0, 0);
    shmsrc = (char*)shmat(shmsrcid, (char*)0, 0);
    resendshm = (char*)shmat(resendshmid, (char*)0, 0);
    sendbackshm = (char*)shmat(sendbackshmid, (char*)0, 0);
    if (shmdest == (char*)-1 || shmsrc == (char*)-1 || resendshm == (char*)-1 || sendbackshm == (char*)-1)
    {
        perror("Failed to attach\n");
        exit(EXIT_FAILURE);
    }

    ops = malloc(sizeof(struct sembuf));                                // used for semaphore operations
    if (ops == NULL) { malloc_error_exit(); }

    msg = malloc(1);                                                    // messages will be stored here before being printed
    if (msg == NULL) { malloc_error_exit(); }
    // Initiallizing msg with a null character so strcmp can be safely called
    msg[0] = '\0';

    // Loop until a termination message has been read
    while ( memcmp(msg, EXIT_MESSAGE, strlen(EXIT_MESSAGE) + 1) != 0 )
    {
        free(msg);

        // Quiting if the operation fails (most likely a termination message has been sent from the opposite direction)
        if ( sem_down(semsrcid, ops, 0) == -1) { quit(SIGQUIT); }

        msg = malloc(strlen(shmsrc) + 1 + MD5_DIGEST_LENGTH);
        if (msg == NULL) { malloc_error_exit(); }

        // Reading from source shared memory
        memcpy(msg, shmsrc, strlen(shmsrc) + 1 + MD5_DIGEST_LENGTH);

        sem_up(semsrcid, ops, 1);

        if ( memcmp(msg, EXIT_MESSAGE, strlen(EXIT_MESSAGE) + 1) == 0)
        // If a termination message has been read, just forward it to destination shared memory
        {
            sem_down(semdestid, ops, 1);
            memcpy(shmdest, msg, strlen(msg) + 1);
            sem_up(semdestid, ops, 0);
        }
        else if (memcmp(msg, RESEND_MESSAGE, strlen(RESEND_MESSAGE) + 1) == 0)
        // A retransmission request has been received, so forward it to `encrypter` instead of `reader`
        {
            // If this operation fails, break instead of calling quit(), so that msg is freed first.
            if (sem_down(sendbacksemid, ops, 1) == -1) { break; }
            memcpy(sendbackshm, msg, strlen(msg) + 1);
            write(STDOUT_FILENO, "Retransmission request received\n", strlen("Retransmission request received\n") + 1);
            sem_up(sendbacksemid, ops, 0);       
        }
        else
        {
            // No special message has been received, so checking for modifications
            if (check_md5(msg, msg + strlen(msg) + 1))
            // No modification was detected, so forward the message to destination shared memory
            {                
                sem_down(semdestid, ops, 1);

                memcpy(shmdest, msg, strlen(msg) + 1);

                sem_up(semdestid, ops, 0);
            }
            else
            // Message modification was detected, so do not forward the message and send a retransmission request
            {
                // If this operation fails, break instead of calling quit(), so that msg is freed first.
                if (sem_down(resendsemid, ops, 1) == -1) { break; }
                memcpy(resendshm, RESEND_MESSAGE, strlen(RESEND_MESSAGE) + 1);
                write(STDOUT_FILENO, "Message corrupted. Asking for retransmission\n", strlen("Message corrupted. Asking for retransmission\n") + 1);
                sem_up(resendsemid, ops, 0);
            }
        }        
    }
    free(msg);
    quit(SIGTERM);
}