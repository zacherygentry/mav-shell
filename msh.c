/*

    Name: Zachery Gentry
    ID:   1001144385

*/

// The MIT License (MIT)
//
// Copyright (c) 2016, 2017, 2018 Trevor Bakker, Zachery Gentry
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

#define MAX_HISTORY 15 // Max number of commands to keep in history and process list

// token and cmd_str used for tokenizing user input
char *token[MAX_NUM_ARGUMENTS];
char cmd_str[MAX_COMMAND_SIZE];

// Will be used to install signal handlers for CtrlZ & Ctrl C
struct sigaction act;

// Pid info kept globally for easy manipulation
pid_t pid;
// This is the pid for msh process
pid_t parent_pid;
pid_t child_pid;
pid_t suspended_process;
// Holds last 15 pids spawned by this shell
pid_t listpids[MAX_HISTORY];

// This counter increments whenever a command is called
int counter = 0;
// Holds last 15 commands used from this shell
char history[MAX_HISTORY][MAX_COMMAND_SIZE];
// Holds the index of the command from history to use. -1 value means the current command is not being pulled from history/
int history_cmd = -1;

// Function initializations, no parametes or return values. Keeping what's needed global instead.
void getInput();
void execute();
void handle_signal();

int main()
{
    // Signal action handler setup
    // Catches ctrl-C & ctrl-Z
    memset(&act, '\0', sizeof(act));
    act.sa_handler = &handle_signal;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTSTP, &act, NULL);

    // Mark the msh process
    parent_pid = getpid();

    //////////////////
    // PROGRAM LOOP //
    //////////////////

    while (1)
    {
        getInput();
        execute();
    }
    return 0;
}

/////////////////////////
/// F U N C T I O N S ///
/////////////////////////

// Global variables used: cmd_str, token
// Places strings into token array, making it usable for execution of commands
//
// Begins shell prompt and listens for user input. Takes input and tokenizes it, placing each string into the token array
// Method written by Professor Bakker. Thank you for this :)
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

    // Tokenize the input strings with whitespace used as the delimiter
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

// Global variables used: token, counter, history, history_cmd, listpids, pid
// Nothing returned. Places commands and processes in history and listpids arrays respectively as they are called
//
// Utilizes the token array by parsing through and attempting to execute user input. Forks the process and searches through multiple directories.
// Custom commands include: listpids (lists all process spawned from this shell), history (shows all previous commands used)
// and !n where n is an integer 0-15 that attempts to execute the corresponding index of history array
void execute()
{
    // If the user just hits enter, do nothing
    if (token[0] == NULL)
    {
        return;
    }
    // User may input 'quit' or 'exit' to get out of the shell
    else if (strcmp(token[0], "quit") == 0 || strcmp(token[0], "exit") == 0)
    {
        exit(EXIT_SUCCESS);
    }
    // User may input cd to change directories. After chdir() is called, returns to listen for input again
    else if (strcmp(token[0], "cd") == 0 && token[1] != NULL)
    {
        // chdir() returns -1 if it fails to find directory
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
    // User may input 'listpids' to list up to 15 processes started from within this shell. Processes start overwriting at index 0 once 15 processes is reached
    else if (strcmp(token[0], "listpids") == 0)
    {
        int i, max;
        // max is dynamic to account for less than 15 processes being available
        max = counter < MAX_HISTORY ? counter : MAX_HISTORY;
        for (i = 0; i < max; i++)
        {
            printf("%d: %d\n", i, listpids[i]);
        }
        return;
    }
    // User may input 'history' to show the last 15 commands used from within this shell. Commands start overwriting at index 0 once 15 commands have been entered
    else if (strcmp(token[0], "history") == 0)
    {
        int i, max;
        // max is dynamic to account for less than 15 commands in history being available
        max = counter < MAX_HISTORY ? counter : MAX_HISTORY;
        for (i = 0; i < max; i++)
        {
            printf("%d: %s\n", i, history[i]);
        }
        return;
    }
    // User may input '!n' where n is an integer 0 through 15 and executes the corresponding command from history.
    else if (token[0][0] == '!')
    {
        char number[MAX_COMMAND_SIZE];
        int i;

        // Adds all tokens to a string 'number'. Starts at i = 1 in order to ignore the '!' char
        for (i = 1; i < sizeof(token[0]); i++)
        {
            number[i - 1] = token[0][i];
        }
        // Converts number string to an integer and assigns history_cmd to it
        if (number[0] != '\0')
        {
            history_cmd = atoi(number);
        }
    }

    // Forks process. Child pid is equal to 0.
    pid = fork();

    child_pid = pid;

    // Since fork has been called, pid must be added to the pid list
    listpids[counter % MAX_HISTORY] = pid;

    // Create command string in order to add to command history list
    char command[MAX_COMMAND_SIZE];
    memset(&command, '\0', MAX_COMMAND_SIZE);

    // Concatenates all usable tokens to a single string and places that command within history array
    int i;
    for (i = 0; i < 10; i++)
    {
        if (token[i] != NULL)
        {
            strcat(command, token[i]);
        }
    }
    // Using % MAX_HISTORY to cycle through array based on a forever incrementing counter
    strcpy(history[counter % MAX_HISTORY], command);

    // This counter increments everytime the process is forked. Used to keep track of number of commands/process started.
    counter++;

    // If this is the child process...
    if (pid == 0)
    {
        // Initialization of directory string. e.g /usr/bin/
        char dir[MAX_COMMAND_SIZE];

        // was a command successfully found? set to 0 if no command found
        int commandFound = 1;

        // If !n was used, this is a command from history
        // Copies the string from history to token 0 and will attempt to be executed
        if (history_cmd != -1)
        {
            strcpy(token[0], history[history_cmd]);
            strcat(token[0], "\0");
            // Must be reset to -1 so that this is no automatically called every iteration
            history_cmd = -1;
        }

        if (strcmp(token[0], "bg") == 0)
        {
            if (suspended_process != '\0')
            {
                kill(suspended_process, SIGCONT);
            }
            exit(EXIT_SUCCESS);
        }

        ///////////////////////////
        /// E X E C U T I O N S ///
        ///////////////////////////

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

    /// END EXECUTIONS
    /// END CHILD PROCESS

    // if the fork failed
    if (pid == -1)
    {
        // When fork() returns -1, an error happened.
        perror("fork failed: ");
        exit(EXIT_FAILURE);
    }

    // Parent process waits here
    else
    {
        int status;
        wait(&status);
        if (WIFSTOPPED(status))
        {
            printf("%d\n", status);
        }
    }
}

// Signal Handler
// Global variables used: parent_pid, pid
// Signal used is passed in as parameter sig
// Catches Ctrl-C and Ctrl-C. If Ctrl-C is used, kill the process if it is NOT the shell. ctrl-Z suspends the process.
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
        suspended_process = child_pid;
    }
}
