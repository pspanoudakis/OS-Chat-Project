#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define DECR_SHM_SOURCE_KEY "2029"
#define DECR_SEM_SOURCE_KEY "2020"
#define DECR_SHM_DEST_KEY "2032"
#define DECR_SEM_DEST_KEY "2017"
#define DECR_SHM_RESEND_KEY "2030"
#define DECR_SEM_RESEND_KEY "2019"
#define DECR_SHM_SENDBACK_KEY "2031"
#define DECR_SEM_SENDBACK_KEY "2018"

#define ENC_SHM_SOURCE_KEY "2031"
#define ENC_SEM_SOURCE_KEY "2018"
#define ENC_SHM_DEST_KEY "2030"
#define ENC_SEM_DEST_KEY "2019"

void handle_child_termination(const int reader, const int writer);

int main(void)
{
    int decrypt_child, encrypt_child;
    char *args[6], *decr_args[10];
    decrypt_child = fork();
    if (decrypt_child == -1)
    {
        perror("Could not fork 1");
        exit(EXIT_FAILURE);
    }
    else if (decrypt_child == 0)
    {
        decr_args[0] = "decrypt";
        decr_args[1] = DECR_SEM_SOURCE_KEY;
        decr_args[2] = DECR_SEM_DEST_KEY;
        decr_args[3] = DECR_SHM_SOURCE_KEY;
        decr_args[4] = DECR_SHM_DEST_KEY;
        decr_args[5] = DECR_SEM_RESEND_KEY;
        decr_args[6] = DECR_SHM_RESEND_KEY;
        decr_args[7] = DECR_SEM_SENDBACK_KEY;
        decr_args[8] = DECR_SHM_SENDBACK_KEY;
        decr_args[9] = NULL;
        execvp("/home/pavlos/vsc/c/OSProject/decrypt", decr_args);
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

void handle_child_termination(const int encrypt, const int decrypt)
{
    int child = wait(NULL);
    
    if (child == 1)
    {
        perror("wait() error");
        exit(EXIT_FAILURE);
    }
    else if (child == encrypt)
    {
        kill(decrypt, SIGQUIT);
    }
    else if (child == decrypt)
    {
        kill(encrypt, SIGQUIT);
    }
}