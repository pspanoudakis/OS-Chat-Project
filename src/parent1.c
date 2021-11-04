/*
 * File: parent1.c
 * Pavlos Spanoudakis (sdi18000184)
 * 
 * Parent process for P1.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/ipc.h>

#define WRITER_SHM_DEST_KEY "2027"
#define WRITTER_SEM_DEST_KEY "2022"
#define READER_SHM_SOURCE_KEY "2025"
#define READER_SEM_SOURCE_KEY "2024"


void handle_child_termination(const int reader, const int writer);

int main(void)
{
    int writer_child, reader_child;
    writer_child = fork();                          // creating a child clone to execute writer
    char *args[4];
    if (writer_child == -1)
    {
        perror("Could not fork 1");
        exit(EXIT_FAILURE);
    }
    else if (writer_child == 0)
    {
        args[0] = "writer";                         // Setting up arguments for `writer`

        key_t key = ftok(".", 1);
        char key_string[2][12];

        sprintf(key_string[0], "%d", key);          // setting up shared memory key argument
        args[1] = key_string[0];

        key = ftok(".", 11);                        // setting up semaphore key argument
        sprintf(key_string[1], "%d", key);
        args[2] = key_string[1];

        args[3] = NULL;

        // The clone replaces itself with writer
        if ( execvp("./writer", args) == -1)
        {
            perror("Failed to execute writer. Aborting.\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        reader_child = fork();                      // creating a child clone to execute reader
        if (reader_child == -1)
        {
            perror("Could not fork 2");
            exit(EXIT_FAILURE);
        }
        else if (reader_child == 0)
        {
            args[0] = "reader";                     // Setting up arguments for `reader`

            key_t key = ftok(".", 2);
            char key_string[2][10];

            sprintf(key_string[0], "%d", key);     // setting up shared memory key argument
            args[1] = key_string[0];

            key = ftok(".", 21);                    // setting up semaphore key argument
            sprintf(key_string[1], "%d", key);
            args[2] = key_string[1];

            args[3] = NULL;

            // The clone replaces itself with reader
            if ( execvp("./reader", args) )
            {
                perror("Failed to execute reader. Aborting.\n");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // Parent will reach this point
            handle_child_termination(reader_child, writer_child);
            exit(EXIT_SUCCESS);
        }        
    }    
}

/* Used by parent to properly handle the children processes. */
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
        kill(writer, SIGTERM);
        wait(NULL);
    }
    else if (child == writer)
    {
        // On the other hand, reader can exit on its own so just wait for it to finish.
        wait(NULL);
    }
}
