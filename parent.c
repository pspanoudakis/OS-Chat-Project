#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

void handle_child_termination(const int reader, const int writer);

int main(void)
{
    int writer_child, reader_child;
    writer_child = fork();
    char *args[4];
    if (writer_child == -1)
    {
        perror("Could not fork 1");
        exit(EXIT_FAILURE);
    }
    else if (writer_child == 0)
    {
        args[0] = "writer";
        args[1] = "2027";
        args[2] = "2022";
        args[3] = NULL;
        execvp("/home/pavlos/vsc/c/OSProject/writer", args);
    }
    else
    {
        reader_child = fork();
        if (reader_child == -1)
        {
            perror("Could not fork 2");
            exit(EXIT_FAILURE);
        }
        else if (reader_child == 0)
        {
            args[0] = "reader";
            args[1] = "2025";
            args[2] = "2024";
            args[3] = NULL;
            execvp("/home/pavlos/vsc/c/OSProject/reader", args);
        }
        else
        {
            handle_child_termination(reader_child, writer_child);
            exit(EXIT_SUCCESS);
        }        
    }    
}

void handle_child_termination(const int reader, const int writer)
{
    int child = wait(NULL);
    
    if (child == -1)
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