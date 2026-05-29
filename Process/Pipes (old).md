# Pipes for shells: `|` is just `pipe()` + `dup2()` + `fork()`

A pipeline like:

```
cmd1 | cmd2
```

means:
- connect **stdout of cmd1** to **stdin of cmd2**

## Headers and prototypes

```c
#include <unistd.h>

int pipe(int pipefd[2]);
```

Return conventions:
- success: `0`
- error: `-1` and sets `errno`

The array meaning is fixed:

```
pipefd[0] = read end
pipefd[1] = write end
```

## The one rule that makes pipelines work: close unused ends

If a process keeps an extra copy of a **write end** open, the reader may never see EOF.
This is the #1 pipeline bug in beginner shells.

Deep explanation (worth reading):
- [Pipes (deep dive ).md](Pipes%20%28deep%20dive%20%29.md)

## Two-command pipeline template (minimal, adapt it)

This is intentionally “template-like” — replace `argv1`/`argv2` with your own argv arrays.

```c
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

static void exec_or_die(char *argv[]) {
    execvp(argv[0], argv);
    perror("execvp");
    _exit(127);
}

int run_pipe2(char *argv1[], char *argv2[]) {
    int p[2];
    if (pipe(p) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t left = fork();
    if (left == -1) {
        perror("fork");
        close(p[0]);
        close(p[1]);
        return -1;
    }
    if (left == 0) {
        /* left child: stdout -> pipe write */
        if (dup2(p[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            _exit(1);
        }
        close(p[0]);
        close(p[1]);
        exec_or_die(argv1);
    }

    pid_t right = fork();
    if (right == -1) {
        perror("fork");
        close(p[0]);
        close(p[1]);
        return -1;
    }
    if (right == 0) {
        /* right child: stdin <- pipe read */
        if (dup2(p[0], STDIN_FILENO) == -1) {
            perror("dup2");
            _exit(1);
        }
        close(p[0]);
        close(p[1]);
        exec_or_die(argv2);
    }

    /* parent: must close both ends too */
    close(p[0]);
    close(p[1]);

    /* parent waits (foreground pipeline) */
    int st1, st2;
    /* For a robust shell: wrap waitpid() in an EINTR-retry loop (see Fork And Wait.md). */
    waitpid(left, &st1, 0);
    waitpid(right, &st2, 0);
    return 0;
}
```

What this gives you (shape):
- You can build `argv1[]` and `argv2[]` from your parser.
- The parent can decide whether to wait or run in background.

What it does *not* do:
- parsing
- job control
- exit-code selection policy (which stage’s status becomes `$?`)

## N-command pipelines (structure hint)

General strategy:
- Keep a variable like `prev_read_end`.
- For each command i:
  - create a new pipe except for the last command
  - in the child: `dup2(prev_read_end, STDIN_FILENO)` (if not first)
  - in the child: `dup2(new_pipe[1], STDOUT_FILENO)` (if not last)
  - close everything you don’t use
- Parent closes ends as it goes.

Don’t code this from memory; derive it carefully from the 2-command version.

## Exercises

1. Break the template by not closing `p[1]` in one process; observe the hang (then explain using write-open counts).
2. Extend from 2 commands to 3 commands.
3. Add support for `cmd1 | cmd2 > out.txt` by combining pipes + redirection.
