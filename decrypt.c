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
char *msg, *shmsrc, *shmdest, *resendshm, *sendbackshm;
struct sembuf *ops;
int semsrcid, shmsrcid, semdestid, shmdestid, resendsemid, resendshmid, sendbacksemid, sendbackshmid;

void sigquit_handler(int signum)
{
    shmdt(shmdest);
    shmdt(shmsrc);
    shmdt(resendshm);
    shmdt(sendbackshm);
    shmctl(shmsrcid, IPC_RMID, 0);
    semctl(semsrcid, 0, IPC_RMID, 0);
    shmctl(shmdestid, IPC_RMID, 0);
    semctl(semdestid, 0, IPC_RMID, 0);
    shmctl(resendshmid, IPC_RMID, 0);
    semctl(resendsemid, 0, IPC_RMID, 0);
    shmctl(sendbackshmid, IPC_RMID, 0);
    semctl(sendbacksemid, 0, IPC_RMID, 0);

    free(ops);
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
    pid_t child_id;
    union semun args;
    signal(SIGQUIT, sigquit_handler);
    // Using 2 semaphores

    key_t sem_source_key = (key_t)atoi(argv[1]);
    key_t sem_dest_key = (key_t)atoi(argv[2]);
    key_t shm_source_key = (key_t)atoi(argv[3]);
    key_t shm_dest_key = (key_t)atoi(argv[4]);
    key_t resend_sem_key = (key_t)atoi(argv[5]);
    key_t resend_shm_key = (key_t)atoi(argv[6]);
    key_t sendback_sem_key = (key_t)atoi(argv[7]);
    key_t sendback_shm_key = (key_t)atoi(argv[8]);

    semdestid = semget(sem_dest_key, 2, IPC_CREAT|PERMS);
    semsrcid = semget(sem_source_key, 2, IPC_CREAT|PERMS);
    resendsemid = semget(resend_sem_key, 2, IPC_CREAT|PERMS);
    sendbacksemid = semget(sendback_sem_key, 2, IPC_CREAT|PERMS);
    if (semdestid == -1 || semsrcid == -1 || resendsemid == -1 || sendbacksemid == -1)
    {
        perror("Failed to get semaphore\n");
        exit(EXIT_FAILURE);
    }

    shmdestid = shmget(shm_dest_key, SHMSIZE, IPC_CREAT|PERMS);
    shmsrcid = shmget(shm_source_key, SHMSIZE, IPC_CREAT|PERMS);
    resendshmid = shmget(resend_shm_key, SHMSIZE, IPC_CREAT|PERMS);
    sendbackshmid = shmget(sendback_shm_key, SHMSIZE, IPC_CREAT|PERMS);
    if (shmdestid == -1 || shmsrcid == -1 || resendshmid == -1 || sendbacksemid == -1)
    {
        perror("Did not get shared memory\n");
        exit(EXIT_FAILURE);
    }

    shmdest = (char*)shmat(shmdestid, (char*)0, 0);
    shmsrc = (char*)shmat(shmsrcid, (char*)0, 0);
    resendshm = (char*)shmat(resendshmid, (char*)0, 0);
    sendbackshm = (char*)shmat(sendbackshmid, (char*)0, 0);
    if (shmdest == (char*)-1 || shmsrc == (char*)-1 || resendshm == (char*)-1 || sendbackshm == (char*)-1)
    {
        perror("Failed to attach\n");
        exit(EXIT_FAILURE);
    }

    ops = malloc(sizeof(struct sembuf));
    msg = malloc(1);
    msg[0] = '\0';
    while ( memcmp(msg, EXIT_MESSAGE, strlen(EXIT_MESSAGE) + 1) != 0 )
    {
        free(msg);
        sem_down(semsrcid, ops, 0);

        msg = malloc(strlen(shmsrc) + 1 + MD5_DIGEST_LENGTH);
        memcpy(msg, shmsrc, strlen(shmsrc) + 1 + MD5_DIGEST_LENGTH);

        sem_up(semsrcid, ops, 1);
        if ( memcmp(msg, EXIT_MESSAGE, strlen(EXIT_MESSAGE) + 1) == 0)
        {
            sem_down(semdestid, ops, 1);
            memcpy(shmdest, msg, strlen(msg) + 1);
            sem_up(semdestid, ops, 0);
        }
        else if (memcmp(msg, RESEND_MESSAGE, strlen(RESEND_MESSAGE) + 1) == 0)
        {
            sem_down(sendbacksemid, ops, 1);
            memcpy(sendbackshm, msg, strlen(msg) + 1);
            write(STDOUT_FILENO, "Informing encrypter\n", strlen("Informing encrypter\n") + 1);
            sem_up(sendbacksemid, ops, 0);       
        }
        else
        {
            if (check_md5(msg, msg + strlen(msg) + 1))
            {                
                sem_down(semdestid, ops, 1);

                memcpy(shmdest, msg, strlen(msg) + 1);

                sem_up(semdestid, ops, 0);
            }
            else
            {
                sem_down(resendsemid, ops, 1);
                memcpy(resendshm, RESEND_MESSAGE, strlen(RESEND_MESSAGE) + 1);
                write(STDOUT_FILENO, "Asking back\n", strlen("Asking back\n") + 1);
                sem_up(resendsemid, ops, 0);
            }
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