/*
 * File: channel_parent.c
 * Pavlos Spanoudakis (sdi18000184)
 * 
 * Parent process for CHAN.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

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

    p1_channel = fork();
    if (p1_channel == -1)
    {
        perror("Could not fork 1");
        exit(EXIT_FAILURE);
    }
    else if (p1_channel == 0)
    {
        args[0] = "channel";
        args[1] = P1_SEM_SOURCE_KEY;
        args[2] = P1_SEM_DEST_KEY;
        args[3] = P1_SHM_SOURCE_KEY;
        args[4] = P1_SHM_DEST_KEY;
        args[5] = argv[1];
        args[6] = NULL;
        execvp("/home/pavlos/vsc/c/OSProject/channel", args);
    }
    else
    {
        p2_channel = fork();
        if (p2_channel == -1)
        {
            perror("Could not fork 2");
            exit(EXIT_FAILURE);
        }
        else if (p2_channel == 0)
        {
            args[0] = "channel";
            args[1] = P2_SEM_SOURCE_KEY;
            args[2] = P2_SEM_DEST_KEY;
            args[3] = P2_SHM_SOURCE_KEY;
            args[4] = P2_SHM_DEST_KEY;
            args[5] = argv[1];
            args[6] = NULL;
            execvp("/home/pavlos/vsc/c/OSProject/channel", args);
        }
        else
        {
            if ( (wait(NULL) == -1) || (wait(NULL) == -1) )
            {
                perror("wait() error");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }        
    }    
}