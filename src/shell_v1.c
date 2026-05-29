#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

int main()
{
    while (1 == 1)
    {
        pid_t pid;
        printf("$ ");
        char *line = NULL;
        size_t len = 0;
        ssize_t read = getline(&line, &len, stdin);

        char *arguments[64];
        const char delimitter[] = " \t\n";

        char *token = strtok(line, delimitter);
        int n = 0;
        while (token)
        {
            arguments[n++] = token;
            token = strtok(NULL, delimitter);
        }
        arguments[n] = NULL;
        if (strcmp(arguments[0], "exit") == 0)
        {
            break;
        }
        else if (strcmp(arguments[0], "cd") == 0)
        {
            chdir(arguments[1]);
        }
        else
        {
            pid = fork();
            if (pid == 0) // child
            {
                execvp(arguments[0], arguments);
                perror("error occured in execvp");
            }
            else // parent
            {
                waitpid(pid, NULL, 0);
                free(line);
                line = NULL;
            }
        }
    }
    return 0;
}
