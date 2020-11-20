#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define WRITER_SHM_DEST_KEY "2027"
#define WRITTER_SEM_DEST_KEY "2022"
#define READER_SHM_SOURCE_KEY "2025"
#define READER_SEM_SOURCE_KEY "2024"

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
        args[1] = WRITER_SHM_DEST_KEY;
        args[2] = WRITTER_SEM_DEST_KEY;
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
            args[1] = READER_SHM_SOURCE_KEY;
            args[2] = READER_SEM_SOURCE_KEY;
            args[3] = NULL;
            execvp("/home/pavlos/vsc/c/OSProject/reader", args);
        }
        else
        {
            handle_child_termination(reader_child, writer_child);
            //wait(NULL);
            //wait(NULL);
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
    // Check which child has terminated
    else if (child == reader)
    {
        // Send a signal to writer, since it is waiting for keyboard input and cannot exit on its own.
        kill(writer, SIGQUIT);
    }
    else if (child == writer)
    {
        // On the other hand, reader can exit on its own so do nothing.
    }
}