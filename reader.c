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

#include "utils.h"

#define SHMSIZE 100
#define PERMS 0660

int semid, shmid;
char* msg, *shmem;
struct sembuf *ops;

void sigquit_handler(int signum)
{
    shmdt(shmem);
    shmctl(shmid, IPC_RMID, 0);
    semctl(semid, 0, IPC_RMID, 0);
    free(ops);
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

    key_t shm_source_key = (key_t)atoi(argv[1]);
    key_t sem_source_key = (key_t)atoi(argv[2]);

    signal(SIGQUIT, sigquit_handler);

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

    shmid = shmget(shm_source_key, SHMSIZE, IPC_CREAT|PERMS);
    if (shmid == -1)
    {
        perror("Did not get shared memory\n");
        exit(EXIT_FAILURE);
    }

    shmem = (char*)shmat(shmid, (char*)0, 0);
    if (shmem == (char*)-1)
    {
        perror("Failed to attach\n");
        exit(EXIT_FAILURE);
    }
    setbuf(stdout, NULL);

    // Child/Consumer process
    ops = malloc(sizeof(struct sembuf));
    msg = malloc(1);
    msg[0] = '\0';
    while (strcmp(msg, EXIT_MESSAGE) != 0)
    {
        free(msg);
        sem_down(semid, ops, 0);

        printf("Just Read: %s\n", shmem);
        msg = malloc(strlen(shmem) + 1);
        strcpy(msg, shmem);
        
        sem_up(semid, ops, 1);
    }
    shmdt(shmem);
    shmctl(shmid, IPC_RMID, 0);
    semctl(semid, 0, IPC_RMID, 0);
    free(msg);
    free(ops);
    exit(EXIT_SUCCESS);
}