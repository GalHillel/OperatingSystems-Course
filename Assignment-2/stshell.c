#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define MAX_ARGS 10
#define MAX_COMMAND_LEN 1024

// Function to handle signals
void handle_signal(int sig)
{
    printf("\n");
}

// Function to execute a command
void execute_command(char **argv, int in_fd, int out_fd)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process
        if (in_fd != STDIN_FILENO)
        {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if (out_fd != STDOUT_FILENO)
        {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        execvp(argv[0], argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else if (pid < 0)
    {
        // Error forking
        perror("fork");
    }
    else
    {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}

int main()
{
    char command[MAX_COMMAND_LEN];
    char *argv[MAX_ARGS];
    char *token;
    int in_fd = STDIN_FILENO;
    int out_fd = STDOUT_FILENO;

    // Register signal handler for Ctrl+C
    signal(SIGINT, handle_signal);

    while (1)
    {
        printf("stShell> ");
        fgets(command, MAX_COMMAND_LEN, stdin);
        command[strlen(command) - 1] = '\0'; // Replace newline with null character

        // Check for exit command
        if (strcmp(command, "exit") == 0)
        {
            break;
        }

        // Parse command line
        int argc = 0;
        token = strtok(command, " ");
        while (token != NULL && argc < MAX_ARGS - 1)
        {
            if (strcmp(token, ">") == 0)
            {
                // Redirect output to file
                token = strtok(NULL, " ");
                out_fd = open(token, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                break;
            }
            else if (strcmp(token, ">>") == 0)
            {
                // Append output to file
                token = strtok(NULL, " ");
                out_fd = open(token, O_WRONLY | O_CREAT | O_APPEND, 0644);
                break;
            }
            else if (strcmp(token, "|") == 0)
            {
                // Create pipe and execute next command
                int pipefd[2];
                pipe(pipefd);
                out_fd = pipefd[1];
                execute_command(argv, in_fd, out_fd);
                in_fd = pipefd[0];
                out_fd = STDOUT_FILENO;
                argc = 0;
            }
            else
            {
                // Add argument to argv
                argv[argc] = token;
                argc++;
            }
            token = strtok(NULL, " ");
        }
        argv[argc] = NULL;

        // Check if there are any commands to execute
        if (argc == 0)
        {
            continue;
        }

        // Execute command with input and output redirection
        execute_command(argv, in_fd, out_fd);

        // Reset input and output file descriptors
        in_fd = STDIN_FILENO;
        out_fd = STDOUT_FILENO;
    }

    return 0;
}
