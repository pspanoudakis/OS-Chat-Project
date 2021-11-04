/*
 * File: encrypter1.c
 * Pavlos Spanoudakis (sdi18000184)
 * 
 * Parent process for ENC1.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>

#define DECR_SHM_SOURCE_KEY "2026"
#define DECR_SEM_SOURCE_KEY "2023"
#define DECR_SHM_DEST_KEY "2025"
#define DECR_SEM_DEST_KEY "2024"
#define DECR_SHM_RESEND_KEY "2028"
#define DECR_SEM_RESEND_KEY "2021"
#define DECR_SHM_SENDBACK_KEY "2027"
#define DECR_SEM_SENDBACK_KEY "2022"

#define ENC_SHM_SOURCE_KEY "2027"
#define ENC_SEM_SOURCE_KEY "2022"
#define ENC_SHM_DEST_KEY "2028"
#define ENC_SEM_DEST_KEY "2021"


int main(void)
{
    int decrypt_child, encrypt_child;
    char *args[6], *decr_args[10];
    decrypt_child = fork();                         // creating a child clone to execute decrypt
    if (decrypt_child == -1)
    {
        perror("Could not fork 1");
        exit(EXIT_FAILURE);
    }
    else if (decrypt_child == 0)
    {
        decr_args[0] = "decrypt";                   // Setting up arguments for `decrypt`
        char key_string[8][12];

        key_t key = ftok(".", 41);
        sprintf(key_string[0], "%d", key);
        decr_args[1] = key_string[0];

        key = ftok(".", 21);                
        sprintf(key_string[1], "%d", key);
        decr_args[2] = key_string[1];

        key = ftok(".", 4);                
        sprintf(key_string[2], "%d", key);
        decr_args[3] = key_string[2];

        key = ftok(".", 2);                
        sprintf(key_string[3], "%d", key);
        decr_args[4] = key_string[3];

        key = ftok(".", 31);                
        sprintf(key_string[4], "%d", key);
        decr_args[5] = key_string[4];

        key = ftok(".", 3);                
        sprintf(key_string[5], "%d", key);
        decr_args[6] = key_string[5];

        key = ftok(".", 11);                
        sprintf(key_string[6], "%d", key);
        decr_args[7] = key_string[6];

        key = ftok(".", 1);                
        sprintf(key_string[7], "%d", key);
        decr_args[8] = key_string[7];

        decr_args[9] = NULL;

        // The clone replaces itself with decrypt
        if ( execvp("./decrypt", decr_args) == -1 )
        {
            perror("Failed to execute decrypt. Aborting.\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        encrypt_child = fork();                     // creating a child clone to execute encrypt
        if (encrypt_child == -1)
        {
            perror("Could not fork 2");
            exit(EXIT_FAILURE);
        }
        else if (encrypt_child == 0)
        {
            args[0] = "encrypt";                    // Setting up arguments for `encrypt`
            char key_string[4][12];

            key_t key = ftok(".", 11);
            sprintf(key_string[0], "%d", key);
            args[1] = key_string[0];

            key = ftok(".", 31);                
            sprintf(key_string[1], "%d", key);
            args[2] = key_string[1];

            key = ftok(".", 1);                
            sprintf(key_string[2], "%d", key);
            args[3] = key_string[2];

            key = ftok(".", 3);                
            sprintf(key_string[3], "%d", key);
            args[4] = key_string[3];

            args[5] = NULL;

            // The clone replaces itself with encrypt
            if ( execvp("./encrypt", args) == -1 )
            {
                perror("Failed to execute encrypt. Aborting.\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // The parent just waits for the 2 children to terminate
            if ( (wait(NULL) == -1) || (wait(NULL) == -1) )
            {
                perror("wait() error");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
    }
}
