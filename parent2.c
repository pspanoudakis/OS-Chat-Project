/*
 * File: parent2.c
 * Pavlos Spanoudakis (sdi18000184)
 * 
 * Parent process for P2.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define WRITER_SHM_DEST_KEY "2031"
#define WRITTER_SEM_DEST_KEY "2018"
#define READER_SHM_SOURCE_KEY "2032"
#define READER_SEM_SOURCE_KEY "2017"


void handle_child_termination(const int reader, const int writer);

int main(void)
{
    int writer_child, reader_child;
    writer_child = fork();                  // creating a child clone to execute writer
    char* args[4];
    if (writer_child == -1)
    {
        perror("Could not fork 1");
        exit(EXIT_FAILURE);
    }
    else if (writer_child == 0)
    {
        args[0] = "writer";                 // setting up required arguments
        args[1] = "2031";
        args[2] = "2018";
        args[3] = NULL;

        // The clone replaces itself with writer
        if ( execvp("/home/pavlos/vsc/c/OSProject/writer", args) == -1)
        {
            perror("Failed to execute writer. Aborting.\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        reader_child = fork();              // creating a child clone to execute reader
        if (reader_child == -1)
        {
            perror("Could not fork 2");
            exit(EXIT_FAILURE);
        }
        else if (reader_child == 0)
        {
            args[0] = "reader";             // setting up required arguments
            args[1] = "2032";
            args[2] = "2017";
            args[3] = NULL;

            // The clone replaces itself with reader
            if ( execvp("/home/pavlos/vsc/c/OSProject/reader", args) )
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
        // On the other hand, reader can exit on its own so do nothing.
        wait(NULL);
    }
}