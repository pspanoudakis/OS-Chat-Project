/*
 * File: channel_parent.c
 * Pavlos Spanoudakis (sdi18000184)
 * 
 * Parent process for CHAN.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ipc.h>

#define P1_SHM_SOURCE_KEY "2028"
#define P1_SEM_SOURCE_KEY "2021"
#define P1_SHM_DEST_KEY "2029"
#define P1_SEM_DEST_KEY "2020"

#define P2_SHM_SOURCE_KEY "2030"
#define P2_SEM_SOURCE_KEY "2019"
#define P2_SHM_DEST_KEY "2026"
#define P2_SEM_DEST_KEY "2023"


int main(int argc, char const *argv[])
{
    int p1_channel, p2_channel;
    char *args[7];
    
    if (argc < 2)
    {
        perror("Insufficient arguments");
        exit(EXIT_FAILURE);
    }

    p1_channel = fork();                                // creating a child clone to execute the first channel (P1->P2)
    if (p1_channel == -1)
    {
        perror("Could not fork 1");
        exit(EXIT_FAILURE);
    }
    else if (p1_channel == 0)
    {
        args[0] = "channel";                            // Setting up arguments for `channel`
        char key_string[4][12];

        key_t key = ftok(".", 31);
        sprintf(key_string[0], "%d", key);
        args[1] = key_string[0];

        key = ftok(".", 51);                
        sprintf(key_string[1], "%d", key);
        args[2] = key_string[1];

        key = ftok(".", 3);                
        sprintf(key_string[2], "%d", key);
        args[3] = key_string[2];

        key = ftok(".", 5);                
        sprintf(key_string[3], "%d", key);
        args[4] = key_string[3];

        args[5] = argv[1];

        args[6] = NULL;

        // The clone replaces itself with channel
        if ( execvp("./channel", args) == -1 )
        {
            perror("Failed to execute channel (1). Aborting.\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        p2_channel = fork();                            // creating a child clone to execute the second channel (P2->P1)
        if (p2_channel == -1)
        {
            perror("Could not fork 2");
            exit(EXIT_FAILURE);
        }
        else if (p2_channel == 0)
        {
            args[0] = "channel";                        // Setting up arguments for `channel`
            char key_string[4][12];

            key_t key = ftok(".", 61);
            sprintf(key_string[0], "%d", key);
            args[1] = key_string[0];

            key = ftok(".", 41);                
            sprintf(key_string[1], "%d", key);
            args[2] = key_string[1];

            key = ftok(".", 6);                
            sprintf(key_string[2], "%d", key);
            args[3] = key_string[2];

            key = ftok(".", 4);                
            sprintf(key_string[3], "%d", key);
            args[4] = key_string[3];

            args[5] = argv[1];

            args[6] = NULL;

            // The clone replaces itself with channel
            if ( execvp("./channel", args) == -1 )
            {
                perror("Failed to execute channel (2). Aborting.\n");
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
