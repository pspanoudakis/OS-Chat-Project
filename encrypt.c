/*
 * File: encrypt.c
 * Pavlos Spanoudakis (sdi18000184)
 * 
 * Child process in ENC1 and ENC2, used for encoding messages.
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

#include "utils.h"

char *msg, *shmsrc, *shmdest;
struct sembuf *ops;
int semsrcid, shmsrcid, semdestid, shmdestid;

void sigquit_handler(int signum)
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
    key_t sem_source_key = (key_t)atoi(argv[1]);
    key_t sem_dest_key = (key_t)atoi(argv[2]);
    key_t shm_source_key = (key_t)atoi(argv[3]);
    key_t shm_dest_key = (key_t)atoi(argv[4]);

    signal(SIGQUIT, sigquit_handler);

    semsrcid = semget(sem_source_key, 2, IPC_CREAT|PERMS);
    semdestid = semget(sem_dest_key, 2, IPC_CREAT|PERMS);
    if (semsrcid == -1 || semdestid == -1)
    {
        perror("Did not get semaphore\n");
        exit(EXIT_FAILURE);
    }

    // Initiallize the first to 0
    //sem_init(semdestid, 0, 0);
    // And the second to 1
    //sem_init(semdestid, 1, 1);

    shmsrcid = shmget(shm_source_key, SHMSIZE, IPC_CREAT|PERMS);
    shmdestid = shmget(shm_dest_key, SHMSIZE, IPC_CREAT|PERMS);
    if (shmsrcid == -1 || shmdestid == -1)
    {
        perror("Did not get shared memory\n");
        exit(EXIT_FAILURE);
    }

    shmsrc = (char*)shmat(shmsrcid, (char*)0, 0);
    shmdest = (char*)shmat(shmdestid, (char*)0, 0);
    if (shmsrc == (char*)-1 || shmdest == (char*)-1)
    {
        perror("Failed to attach\n");
        exit(EXIT_FAILURE);
    }

    // Child/Consumer process
    ops = malloc(sizeof(struct sembuf));
    if (ops == NULL) { malloc_error_exit(); }

    msg = malloc(1);
    if (msg == NULL) { malloc_error_exit(); }
    msg[0] = '\0';

    while (strcmp(msg, EXIT_MESSAGE) != 0)
    {
        free(msg);
        if (sem_down(semsrcid, ops, 0) == -1) { sigquit_handler(SIGQUIT); }

        msg = malloc(strlen(shmsrc)+1);
        if (msg == NULL) { malloc_error_exit(); }
        strcpy(msg, shmsrc);

        sem_up(semsrcid, ops, 1);

        if (sem_down(semdestid, ops, 1) == -1) { sigquit_handler(SIGQUIT); }
        if (strcmp(msg, EXIT_MESSAGE) == 0)
        {
            strcpy(shmdest, msg);
        }
        else if (strcmp(msg, RESEND_MESSAGE) == 0)
        {
            write(STDOUT_FILENO, "Retransmitting last message\n", strlen("Retransmitting last message\n") + 1);
            // "retransmit" the last message wrote in shared memory.
            // the shared memory still contains this message, so no need to re-write it.
        }        
        else
        {
            memcpy(shmdest, msg, strlen(msg) + 1);
            hashed = md5_hash(msg);
            memcpy(shmdest + strlen(msg) + 1, hashed, MD5_DIGEST_LENGTH);
            free(hashed);
        }
        sem_up(semdestid, ops, 0);
    }
    shmdt(shmsrc);
    shmctl(shmsrcid, IPC_RMID, 0);
    semctl(semsrcid, 0, IPC_RMID, 0);
    shmdt(shmdest);
    shmctl(shmdestid, IPC_RMID, 0);
    semctl(semdestid, 0, IPC_RMID, 0);
    free(msg);
    free(ops);
    exit(EXIT_SUCCESS);
}