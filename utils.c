/* 
 * File: utils.h
 * Pavlos Spanoudakis (sdi1800184)
 * 
 * Contains implementations for useful and widely used functions.
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
#include <openssl/md5.h>

#include "utils.h"

/* To be used in case a malloc call fails. */
void malloc_error_exit(void)
{
    perror("Malloc failed\n");
    exit(EXIT_FAILURE);
}

/* Reads a character sequence up to a newline character, and gradually
   stores it in *buffer. This is helpful in case of a sudden termination of the calling process:
   a signal handler can free(*buffer) to prevent a memory leak.*/
void get_line(char **buffer)
{
    int c, count;
    
    *buffer = malloc(1);                                        // The string will be stored directly to the specified buffer
    if (buffer == NULL)  { malloc_error_exit(); }
    (*buffer)[0] = '\0';                                        // First, initiallizing it as empty

    count = 1;                                                  // Counting already allocated bytes
    while ( (c = getchar()) != '\n' )
    {  
        *buffer = (char*)realloc(*buffer, count + 1);           // Increasing the space by 1 byte
        (*buffer)[count - 1] = c;                               // Storing the new letter
        (*buffer)[count] = '\0';
        count++;
    }
}

/* Checks if the specified hashcode is the correct MD5 hashcode
   for the specified character sequence. */
int check_md5(const char *data, const char *hash)
{
    char *md5_hash;
    int valid = 0;

    md5_hash = malloc(MD5_DIGEST_LENGTH);
    if (md5_hash == NULL) { malloc_error_exit(); }

    MD5(data, strlen(data) + 1, md5_hash);                      // Finding the hashcode for the specified data

    if ( memcmp(md5_hash, hash, MD5_DIGEST_LENGTH) == 0 )       // And comparing it with the given one
    {
        valid = 1;
    }
    
    free(md5_hash);
    return valid;
}

/* Returns a character sequence with the MD5 hashcode of the specified data. */
char* md5_hash(const char *data)
{
    char *hashcode;

    hashcode = malloc(MD5_DIGEST_LENGTH);
    if (hashcode == NULL) { malloc_error_exit(); }

    MD5(data, strlen(data) + 1, hashcode);

    return hashcode;
}

/* Randomly modifies the specified character sequence.
   The null terminator is left unchanged.*/
void add_noise(char *data)
{
    int length = strlen(data);
    if (length == 0) { return; }            // An empty string will not be modifed

    int n = (rand() % length) + 1;          // Number of times a character will be replaced (at least 1)
    int pos;                                // The index of the character to be replaced

    for (int i = 1; i <= n; i++)            // i just counts how many times a character has been replaced
    {
        pos = rand() % length;              // Choose a random character to replace (null terminator will never be chosen)
        data[pos] = (rand() % 94) + 33;     // Replace with a printable character (ASCII 33-126)
    }
}

/* Initiallizes the semaphore with the specified index to the specified value. */
int sem_init(int semid, int semnum, int value)
{
    union semun args;
    args.val = value;
    return semctl(semid, semnum, SETVAL, args);
}

/* Unlocks/Increments the semaphore, using the operation buffer semops.
   semops must be a valid pointer to a sembuf. */
int sem_up(int semid, struct sembuf *semops, int semnum)
{
    semops->sem_flg = 0;                    // setting up sembuf properly
    semops->sem_num = semnum;
    semops->sem_op = 1;

    return semop(semid, semops, 1);         // semaphore up
}

/* Locks/Decrements the semaphore, using the operation buffer semops.
   semops must be a valid pointer to a sembuf. */
int sem_down(int semid, struct sembuf *semops, int semnum)
{
    semops->sem_flg = 0;                    // setting up sembuf properly
    semops->sem_num = semnum;
    semops->sem_op = -1;
    
    return semop(semid, semops, 1);         // semaphore down
}