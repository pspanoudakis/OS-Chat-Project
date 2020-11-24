/*
 * File: encrypt.c
 * Pavlos Spanoudakis (sdi18000184)
 * 
 * Child process in ENC1 and ENC2, used for encoding messages.
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

#include "utils.h"

// Global declerations (in order to be visible from quit())
char *msg, *shmsrc, *shmdest;
struct sembuf *ops;
int semsrcid, shmsrcid, semdestid, shmdestid;

/* Used for Terminating properly (freeing heap memory, dettaching pointers 
& deleting semaphores and shared memory segments) */
void quit(int signum)
{
    shmdt(shmsrc);
    shmctl(shmsrcid, IPC_RMID, 0);
    semctl(semsrcid, 0, IPC_RMID, 0);
    shmdt(shmdest);
    shmctl(shmdestid, IPC_RMID, 0);
    semctl(semdestid, 0, IPC_RMID, 0);
    free(ops);
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
    pid_t child_id;
    union semun args;
    char *hashed;

    if (argc < 5)
    {
        perror("Insufficient arguments\n");
        exit(EXIT_FAILURE);
    }
    // Getting semaphore and shared memory keys
    key_t sem_source_key = (key_t)atoi(argv[1]);
    key_t sem_dest_key = (key_t)atoi(argv[2]);
    key_t shm_source_key = (key_t)atoi(argv[3]);
    key_t shm_dest_key = (key_t)atoi(argv[4]);

    // To terminate smoothly if a SIGTERM is sent
    signal(SIGTERM, quit);

    semsrcid = semget(sem_source_key, 2, IPC_CREAT|PERMS);                      // 2 semaphores to handle shared memory for reading
    semdestid = semget(sem_dest_key, 2, IPC_CREAT|PERMS);                       // 2 semaphores to handle shared memory for writing
    if (semsrcid == -1 || semdestid == -1)
    {
        perror("Did not get semaphore\n");
        exit(EXIT_FAILURE);
    }
    // Note that the semaphores should have been created and initiallized before this process starts
    // (parent1, parent2 and channel_parent *must* have been called first)

    shmsrcid = shmget(shm_source_key, SHMSIZE, IPC_CREAT|PERMS);                // Aquiring shared memory for reading
    shmdestid = shmget(shm_dest_key, SHMSIZE, IPC_CREAT|PERMS);                 // Aquiring shared memory for writing
    if (shmsrcid == -1 || shmdestid == -1)
    {
        perror("Did not get shared memory\n");
        exit(EXIT_FAILURE);
    }

    // Attaching pointers to shared memory segments
    shmsrc = (char*)shmat(shmsrcid, (char*)0, 0);
    shmdest = (char*)shmat(shmdestid, (char*)0, 0);
    if (shmsrc == (char*)-1 || shmdest == (char*)-1)
    {
        perror("Failed to attach\n");
        exit(EXIT_FAILURE);
    }

    ops = malloc(sizeof(struct sembuf));                                        // used for semaphore operations
    if (ops == NULL) { malloc_error_exit(); }

    msg = malloc(1);                                                            // messages will be stored here for processing and forwarding
    if (msg == NULL) { malloc_error_exit(); }
    // Initiallizing msg with a null character so strcmp can be safely called
    msg[0] = '\0';

    // Loop until a termination message has been read
    while (strcmp(msg, EXIT_MESSAGE) != 0)
    {
        free(msg);

        // Quiting if the operation fails (most likely a termination message has been sent from the opposite direction)
        if (sem_down(semsrcid, ops, 0) == -1) { quit(SIGTERM); }

        msg = malloc(strlen(shmsrc)+1);
        if (msg == NULL) { malloc_error_exit(); }

        // Reading from source shared memory
        strcpy(msg, shmsrc);

        sem_up(semsrcid, ops, 1);

        // If this operation fails, break instead of calling quit(), so that msg is freed first.
        if (sem_down(semdestid, ops, 1) == -1) { break; }

        // Now ready to write to destination shared memory
        if (strcmp(msg, EXIT_MESSAGE) == 0)
        // If this is a termination message, just forward it without encryption
        {
            strcpy(shmdest, msg);
        }
        else if (strcmp(msg, RESEND_MESSAGE) == 0)
        // This is a retransmission message
        {
            write(STDOUT_FILENO, "Retransmitting last message\n", strlen("Retransmitting last message\n") + 1);
            // "retransmiting" the last message wrote in shared memory.
            // the shared memory still contains this message, so no need to re-write it.
        }        
        else
        // No special message has been received
        {
            // First write the original message to shared memory
            memcpy(shmdest, msg, strlen(msg) + 1);

            hashed = md5_hash(msg);
            // And also write the hashcode of it
            memcpy(shmdest + strlen(msg) + 1, hashed, MD5_DIGEST_LENGTH);
            free(hashed);
        }
        sem_up(semdestid, ops, 0);
    }
    free(msg);
    quit(SIGTERM);
}