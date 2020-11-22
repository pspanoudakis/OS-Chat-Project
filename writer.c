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

struct sembuf *ops;
int semid, shmid;
char *msg, *shmem;

void sigquit_handler(int signum)
{
    shmdt(shmem);
    shmctl(shmid, IPC_RMID, 0);
    semctl(semid, 0, IPC_RMID, 0);
    free(msg);
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
    key_t shm_dest_key = (key_t)atoi(argv[1]);
    key_t sem_dest_key = (key_t)atoi(argv[2]);

    signal(SIGQUIT, sigquit_handler);

    // Using 2 semaphores
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

    shmid = shmget(shm_dest_key, SHMSIZE, IPC_CREAT|PERMS);
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
    // Make stdout unbuffered
    setbuf(stdout, NULL);

    // Parent/Producer process
    ops = malloc(sizeof(struct sembuf));
    if (ops == NULL) { malloc_error_exit(); }

    msg = malloc(1);
    if (msg == NULL) { malloc_error_exit(); }
    msg[0] = '\0';

    while ( strcmp(msg, EXIT_MESSAGE) != 0 )
    {
        free(msg);
        get_line(&msg);
        if (strlen(msg) > (SHMSIZE - MD5_DIGEST_LENGTH - 1))
        {
            printf("Message must be up to %d characters.", (SHMSIZE - MD5_DIGEST_LENGTH - 1));
            msg = malloc(1);
            if (msg == NULL) { malloc_error_exit(); }
            msg[0] = '\0';
            continue;
        }
        if (sem_down(semid, ops, 1) == -1){ continue; }
        
        printf("Writting: %s\n", msg);
        strcpy(shmem, msg);

        sem_up(semid, ops, 0);
    }
    shmdt(shmem);
    shmctl(shmid, IPC_RMID, 0);
    semctl(semid, 0, IPC_RMID, 0);
    free(msg);
    free(ops);
    exit(EXIT_SUCCESS);
}