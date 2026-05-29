
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

    while (1)
    {
        printf("-> ");
        char *line = NULL;
        size_t len = 0;
        ssize_t read = getline(&line, &len, stdin);
        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }
        printf("cmd : %s\n",line);
        if (strcmp(line, "exit") == 0)
            break;
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

        pid_t pid = fork();
        if (pid == 0) // child
        {
            execvp(arguments[0], arguments);
            perror("error occured in execvp");
        }
        else // parent
        {
            waitpid(pid, NULL, 0);
            // printf("Child exited\n");
        }
    }

    return 0;
}