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

#define SHMSIZE 100
#define PERMS 0660

// Global declerations (in order to be visible from the handler)
char *msg, *shmsrc, *shmdest;
struct sembuf *ops;
int semsrcid, shmsrcid, semdestid, shmdestid;

void sigquit_handler(int signum)
{
    shmdt(shmdest);
    shmdt(shmsrc);
    shmctl(shmsrcid, IPC_RMID, 0);
    semctl(shmsrcid, 0, IPC_RMID, 0);
    shmctl(shmdestid, IPC_RMID, 0);
    semctl(semdestid, 0, IPC_RMID, 0);
    free(ops);
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
    pid_t child_id;
    union semun args;
    signal(SIGQUIT, sigquit_handler);
    char* temp;
    // Using 2 semaphores

    key_t sem_source_key = (key_t)atoi(argv[1]);
    key_t sem_dest_key = (key_t)atoi(argv[2]);
    key_t shm_source_key = (key_t)atoi(argv[3]);
    key_t shm_dest_key = (key_t)atoi(argv[4]);
    int chance = atoi(argv[5]);

    semdestid = semget(sem_dest_key, 2, IPC_CREAT|PERMS);
    semsrcid = semget(sem_source_key, 2, IPC_CREAT|PERMS);
    if (shmsrc == (char*)-1 || shmdest == (char*)-1)
    {
        perror("Failed to attach\n");
        exit(EXIT_FAILURE);
    }

    // Semaphores to handle shared memory for writting
    sem_init(semdestid, 0, 0);
    sem_init(semdestid, 1, 1);
    // Semaphores to handle shared memory for reading
    sem_init(semsrcid, 0, 0);
    sem_init(semsrcid, 1, 1);

    shmdestid = shmget(shm_dest_key, SHMSIZE, IPC_CREAT|PERMS);
    shmsrcid = shmget(shm_source_key, SHMSIZE, IPC_CREAT|PERMS);
    if (shmdestid == -1)
    {
        perror("Did not get shared memory\n");
        exit(EXIT_FAILURE);
    }
    shmdest = (char*)shmat(shmdestid, (char*)0, 0);
    shmsrc = (char*)shmat(shmsrcid, (char*)0, 0);
    if (shmdest == (char*)-1)
    {
        perror("Failed to attach\n");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    ops = malloc(sizeof(struct sembuf));
    msg = malloc(1);
    msg[0] = '\0';
    while ( memcmp(msg, EXIT_MESSAGE, strlen(EXIT_MESSAGE) + 1) != 0 )
    {
        free(msg);
        // Reading from source shared memory
        sem_down(semsrcid, ops, 0);

        msg = malloc(strlen(shmsrc) + 1 + MD5_DIGEST_LENGTH);
        memcpy(msg, shmsrc, strlen(shmsrc) + 1 + MD5_DIGEST_LENGTH);

        sem_up(semsrcid, ops, 1);
        if ( (memcmp(msg, EXIT_MESSAGE, strlen(EXIT_MESSAGE) + 1) != 0) 
            && (memcmp(msg, RESEND_MESSAGE, strlen(RESEND_MESSAGE) + 1) != 0) )
        {
            // Writting to destination shared memory
            sem_down(semdestid, ops, 1);
            memcpy(shmdest, msg, strlen(msg) + 1 + MD5_DIGEST_LENGTH);
            if ((rand() % 100) < chance)
            {
                //write(STDOUT_FILENO, "Noise Added\n", strlen("Noise Added\n"));
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