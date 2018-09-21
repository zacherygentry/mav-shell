/*

    Name: Zachery Gentry
    ID:   1001144385

*/

// The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n" // We want to split our command line up into tokens 
                           // so we need to define what delimits our tokens.   
                           // In this case  white space                        
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10 // Mav shell only supports five arguments

char *token[MAX_NUM_ARGUMENTS];

void getInput(char *);
void execute();

int main()
{
    char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);

    while (1)
    {
        getInput(cmd_str);
        execute();
    }
    return 0;
}

void getInput(char *cmd_str)
{
    // Print out the msh prompt
    printf("msh> ");
    int i;
    for (i = 0; i < MAX_NUM_ARGUMENTS; i++)
    {
        token[i] = NULL;
    }

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin))
        ;

    /* Parse input */

    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str = strdup(cmd_str);

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    {
        token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
        if (strlen(token[token_count]) == 0)
        {
            token[token_count] = NULL;
        }
        token_count++;
    }

    free(working_root);
}

void execute()
{
    if (token[0] == NULL)
    {
    }
    else if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    pid_t pid = fork();

    if (pid == 0)
    {
        char dir[MAX_COMMAND_SIZE];
        int commandFound = 1;

        strcpy(dir, "./");
        strcat(dir, token[0]);
        if (execl(dir, token[0], token[1], token[2], token[3], token[4], token[5], token[6], token[7], token[8], token[9], NULL))
        {
            commandFound = 0;
        }

        strcpy(dir, "/usr/local/bin/");
        strcat(dir, token[0]);
        if (execl(dir, token[0], token[1], token[2], token[3], token[4], token[5], token[6], token[7], token[8], token[9], NULL))
        {
            commandFound = 0;
        }

        strcpy(dir, "/usr/bin/");
        strcat(dir, token[0]);
        if (execl(dir, token[0], token[1], token[2], token[3], token[4], token[5], token[6], token[7], token[8], token[9], NULL))
        {
            commandFound = 0;
        }

        strcpy(dir, "/bin/");
        strcat(dir, token[0]);
        if (execl(dir, token[0], token[1], token[2], token[3], token[4], token[5], token[6], token[7], token[8], token[9], NULL))
        {
            commandFound = 0;
        }

        if (!commandFound)
        {
            printf("%s: Command not found.\n", token[0]);
        }

        exit(EXIT_SUCCESS);
    }

    else
    {
        wait(&pid);
    }
}
