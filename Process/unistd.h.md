# `unistd.h`: the POSIX header that defines process and I/O syscalls

## What is unistd.h? A history and purpose

The name `unistd.h` is short for "**Unix Standard**". It's a POSIX (Portable Operating System Interface) header file that declares functions for:

- Process creation and control (`fork()`, `exec*()`, `_exit()`)
- File I/O and file descriptor operations (`read()`, `write()`, `close()`, `dup2()`)
- Pipe creation (`pipe()`)
- Process introspection (`getpid()`, `getppid()`)
- File system navigation (`chdir()`, `getcwd()`)
- Terminal checks (`isatty()`)

In essence, `unistd.h` gives you the low-level Unix/Linux syscall interface for working with **processes, files, and inter-process communication**.

### Why "unistd.h" and not just "process.h"?

The name reflects Unix's design philosophy: **everything is a file**. A process, a pipe, a terminal, a regular file on disk—they all behave similarly through a unified I/O interface. `unistd.h` provides both process syscalls and I/O syscalls because they're deeply intertwined.

## What you need from unistd.h for a shell

For your shell project, only a few syscalls from `unistd.h` are essential:

### Process creation and termination

```c
#include <unistd.h>

pid_t fork(void);                  /* Create a child process. */
int execve(const char *path, char *const argv[], 
           char *const envp[]);    /* Replace process with new program. */
int execvp(const char *file, char *const argv[]);  /* Like execve, but search PATH. */
void _exit(int status);            /* Exit immediately (no cleanup). */
```

- **`fork()`** creates a duplicate of the calling process. Returns 0 in child, child PID in parent, -1 on error.
- **`execve()` and `execvp()`** replace the calling process with a new program. Never return on success.
- **`_exit()`** terminates the process immediately. Used in the child after a failed `exec()`.

### File descriptor operations

```c
#include <unistd.h>

ssize_t read(int fd, void *buf, size_t count);    /* Read from fd into buffer. */
ssize_t write(int fd, const void *buf, size_t count);  /* Write buffer to fd. */
int close(int fd);                 /* Close a file descriptor. */
int dup(int oldfd);                /* Duplicate fd (returns new fd). */
int dup2(int oldfd, int newfd);    /* Duplicate fd to specific number. */
```

- **`read()/write()`** move data. Return byte count or -1.
- **`close()`** closes a descriptor and decrements reference counts.
- **`dup2()`** is your main tool for **redirection**: making one fd point to another's underlying file.

### Pipes

```c
#include <unistd.h>

int pipe(int pipefd[2]);           /* Create a unidirectional data channel. */
```

- Returns 0 on success, -1 on error.
- Fills `pipefd[0]` (read end) and `pipefd[1]` (write end).

### Miscellaneous process queries

```c
pid_t getpid(void);                /* Get current process's PID. */
pid_t getppid(void);               /* Get parent process's PID. */
pid_t getpgrp(void);               /* Get current process group. */
int isatty(int fd);                /* Is fd connected to a terminal? */
```

- **`isatty(STDIN_FILENO)`** tells you if the shell is interactive (true) or scripted (false).
- **`getpgrp()`** is used in job control to manage process groups.

## Other headers you'll need alongside unistd.h

`unistd.h` is not enough by itself. You'll almost always include:

### `<sys/wait.h>`

Defines `waitpid()`, `wait()`, and status macros (`WIFEXITED`, `WEXITSTATUS`, `WIFSIGNALED`, etc.):

```c
#include <sys/wait.h>

pid_t waitpid(pid_t pid, int *status, int options);
```

This is how the parent collects a child's exit status and reaps zombies. It's as essential as `fork()`.

### `<fcntl.h>`

Defines `open()` and file access flags:

```c
#include <fcntl.h>

int open(const char *pathname, int flags, mode_t mode);
```

Flags include `O_RDONLY`, `O_WRONLY`, `O_CREAT`, `O_TRUNC`, `O_APPEND`, `O_CLOEXEC`.

### `<errno.h>`

Defines `errno` and `EINTR`. After any syscall that might fail:

```c
#include <errno.h>

if (syscall() == -1) {
    if (errno == EINTR) {
        /* Interrupted by a signal; retry. */
    } else {
        perror("syscall");
    }
}
```

### `<signal.h>`

Defines signal handling functions (`signal()`, `sigaction()`) and signal numbers (`SIGCHLD`, `SIGINT`, etc.):

```c
#include <signal.h>

void (*signal(int sig, void (*handler)(int)))(int);
```

Used for handling Ctrl-C, reaping background children, etc.

### `<string.h>`

Defines `strerror()`, which converts `errno` to a human-readable message:

```c
#include <string.h>

char *strerror(int errnum);
```

Used in error messages.

### `<sys/types.h>`

Defines types like `pid_t`, `size_t`, `mode_t`:

```c
#include <sys/types.h>
```

Usually included implicitly by other headers, but good practice to include it explicitly.

## The include pattern for a minimal shell

Here's what a shell typically includes at the top:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
```

You might add others as you implement more features:
- `<sys/stat.h>` for file mode introspection
- `<termios.h>` for terminal control (job control)
- `<pwd.h>` for password database queries (username expansion)

## Manpages: your best friend for syscall details

When you need to know the exact behavior of a syscall, use the manpages:

```bash
man 2 fork          # Section 2 = system calls
man 2 execve
man 2 waitpid
man 2 pipe
man 2 dup2
man 2 open
man 2 read
man 2 write
```

The manpages include:
- **SYNOPSIS**: exact prototype.
- **DESCRIPTION**: detailed semantics.
- **RETURN VALUE**: what the function returns on success and failure.
- **ERRORS**: what values `errno` can be set to.
- **EXAMPLES**: often tiny code snippets.
- **SEE ALSO**: related syscalls.

For a shell, the **RETURN VALUE** and **ERRORS** sections are your lifeline.

## Portability: unistd.h is POSIX, not Windows

`unistd.h` is available on:
- Linux
- macOS
- BSD variants
- POSIX-compliant Unix systems

It is **not available on Windows** (Windows uses `<windows.h>` instead, with completely different APIs).

Your shell will be Unix/Linux-only. That's fine; shells are inherently Unix concepts.

## Why separate syscalls instead of a unified function?

Unix designers had a philosophy: **do one thing well**. Instead of a single `run_command()` function, there are separate `fork()`, `exec()`, `pipe()`, `dup2()`, `waitpid()` functions. This separation means:

- You can fork without executing (for background jobs).
- You can exec without forking (replace yourself).
- You can set up pipes and redirections between fork and exec.
- You can wait for a specific child, or any child, or non-blocking.

This flexibility is why Unix shells are powerful. A monolithic function would be simpler to learn, but much less flexible.

## Key takeaway

`#include <unistd.h>` plus `<sys/wait.h>`, `<fcntl.h>`, `<signal.h>` are your core set. Together, they provide the syscall interface for building a shell. The manpages are your reference for details. Start with simple patterns (fork + exec + wait), then add complexity (pipes, redirections, job control) as needed.
