#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define DECR_SHM_SOURCE_KEY "2026"
#define DECR_SEM_SOURCE_KEY "2023"
#define DECR_SHM_DEST_KEY "2025"
#define DECR_SEM_DEST_KEY "2024"

#define ENC_SHM_SOURCE_KEY "2027"
#define ENC_SEM_SOURCE_KEY "2022"
#define ENC_SHM_DEST_KEY "2028"
#define ENC_SEM_DEST_KEY "2021"

void handle_child_termination(const int reader, const int writer);

int main(void)
{
    int decrypt_child, encrypt_child;
    char *args[6];
    decrypt_child = fork();
    if (decrypt_child == -1)
    {
        perror("Could not fork 1");
        exit(EXIT_FAILURE);
    }
    else if (decrypt_child == 0)
    {
        args[0] = "decrypt";
        args[1] = DECR_SEM_SOURCE_KEY;
        args[2] = DECR_SEM_DEST_KEY;
        args[3] = DECR_SHM_SOURCE_KEY;
        args[4] = DECR_SHM_DEST_KEY;
        args[5] = NULL;
        execvp("/home/pavlos/vsc/c/OSProject/decrypt", args);
    }
    else
    {
        encrypt_child = fork();
        if (encrypt_child == -1)
        {
            perror("Could not fork 2");
            exit(EXIT_FAILURE);
        }
        else if (encrypt_child == 0)
        {
            args[0] = "encrypt";
            args[1] = ENC_SEM_SOURCE_KEY;
            args[2] = ENC_SEM_DEST_KEY;
            args[3] = ENC_SHM_SOURCE_KEY;
            args[4] = ENC_SHM_DEST_KEY;
            args[5] = NULL;
            execvp("/home/pavlos/vsc/c/OSProject/encrypt", args);
        }
        else
        {
            handle_child_termination(encrypt_child, decrypt_child);
            exit(EXIT_SUCCESS);
        }        
    }    
}

void handle_child_termination(const int reader, const int writer)
{
    int child = wait(NULL);
    
    if (child == 1)
    {
        perror("wait() error");
        exit(EXIT_FAILURE);
    }
    else if (child == reader)
    {
        kill(writer, SIGQUIT);
    }
    else if (child == writer)
    {
        kill(reader, SIGQUIT);
    }
}