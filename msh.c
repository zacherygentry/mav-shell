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

#define WHITESPACE " \t\n" // We want to split our command line up into tokens \
                           // so we need to define what delimits our tokens.   \
                           // In this case  white space                        \
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10 // Mav shell only supports five arguments

#define MAX_HISTORY 15

char *token[MAX_NUM_ARGUMENTS];
char cmd_str[MAX_COMMAND_SIZE];
struct sigaction act;
pid_t pid;
pid_t parent_pid;
pid_t last_process;
pid_t listpids[MAX_HISTORY];
int counter = 0;
char history[MAX_HISTORY][MAX_COMMAND_SIZE];
int history_cmd = -1;

void getInput();
void execute();
void handle_signal();

int main()
{
    memset(&act, '\0', sizeof(act));
    act.sa_handler = &handle_signal;

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTSTP, &act, NULL);
    sigaction(SIGALRM, &act, NULL);

    parent_pid = getpid();

    while (1)
    {
        getInput();
        execute();
    }
    return 0;
}

void getInput()
{
    printf("msh> ");

    memset(cmd_str, '\0', MAX_COMMAND_SIZE);
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

    memset(&token, '\0', MAX_NUM_ARGUMENTS);
    // Tokenize the input stringswith whitespace used as the delimiter
    memset(&token, '\0', sizeof(MAX_NUM_ARGUMENTS));
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
        return;
    }
    else if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(token[0], "cd") == 0 && token[1] != NULL)
    {
        int status = chdir(token[1]);
        if (status == -1)
        {
            printf("No such file or directory named %s.\n", token[1]);
            return;
        }
        else
        {
            return;
        }
    }
    else if (strcmp(token[0], "listpids") == 0)
    {
        int i, max;
        max = counter < MAX_HISTORY ? counter : MAX_HISTORY;
        for (i = 0; i < max; i++)
        {
            printf("%d: %d\n", i, listpids[i]);
        }
        return;
    }
    else if (strcmp(token[0], "history") == 0)
    {
        int i, max;
        max = counter < MAX_HISTORY ? counter : MAX_HISTORY;
        for (i = 0; i < max; i++)
        {
            printf("%d: %s\n", i, history[i]);
        }
        return;
    }
    else if (token[0][0] == '!')
    {
        char number[MAX_COMMAND_SIZE];
        int i;

        for (i = 1; i < sizeof(token[0]); i++)
        {
            number[i - 1] = token[0][i];
        }
        if (number[0] != '\0')
        {
            history_cmd = atoi(number);
        }
    }
    pid = fork();
    listpids[counter % MAX_HISTORY] = pid;

    char command[MAX_COMMAND_SIZE];
    memset(&command, '\0', MAX_COMMAND_SIZE);
    int i;
    for (i = 0; i < 10; i++)
    {
        if (token[i] != NULL)
        {
            printf("Token %d: %s", i, token[i]);
            strcat(command, token[i]);
        }
    }
    printf("%s\n", command);
    strcpy(history[counter % MAX_HISTORY], command);
    counter++;

    if (pid == 0)
    {
        char dir[MAX_COMMAND_SIZE];
        int commandFound = 1;

        if (history_cmd != -1)
        {
            strcpy(token[0], history[history_cmd]);
            strcat(token[0], "\0");
            history_cmd = -1;
        }

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

    if (pid == -1)
    {
        // When fork() returns -1, an error happened.
        perror("fork failed: ");
        exit(EXIT_FAILURE);
    }
    else
    {
        wait(&pid);
    }
}

void handle_signal(int sig)
{
    // CTRL C
    if (sig == SIGINT)
    {
        if (getpid() != parent_pid)
        {
            kill(pid, SIGKILL);
        }
        wait(&pid);
    }

    // CTRL Z
    if (sig == SIGTSTP)
    {
        if (getpid() != parent_pid)
        {
        }
    }
}