# `unistd.h` (POSIX): the header you’ll live in

If you’re building a shell on Linux, `#include <unistd.h>` shows up everywhere.
It declares many POSIX functions for **process**, **file descriptor**, and **pipe** work.

This note is a quick “what lives where” map so you know which headers to include.

## The shell-related essentials

### Process creation + program replacement

```c
#include <unistd.h>

pid_t fork(void);
int execve(const char *pathname, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);
void _exit(int status);
```

- `fork()` returns `0` in the child, `>0` (child PID) in the parent, `-1` on error.
- `exec*()` returns only on error (`-1`) and sets `errno`.
- `_exit()` terminates a process immediately (use in the child when you must exit after a failed `exec`).

Read next:
- [Fork And Wait.md](Fork%20And%20Wait.md)
- [Exec.md](Exec.md)

### File descriptors (FDs): read/write/close/dup

```c
#include <unistd.h>

ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);

int dup(int oldfd);
int dup2(int oldfd, int newfd);
```

- `read()`/`write()` return a byte count `>= 0`, or `-1` on error.
- `dup2(oldfd, newfd)` is the core syscall for **redirection**.

Read next:
- [dup() and dup2().md](dup%28%29%20and%20dup2%28%29.md)

### Pipes

```c
#include <unistd.h>

int pipe(int pipefd[2]);
```

- On success, `pipefd[0]` is **read end**, `pipefd[1]` is **write end**.

Read next:
- [Pipes.md](Pipes.md)

### Terminal checks (interactive shells)

```c
#include <unistd.h>

int isatty(int fd);
pid_t getpgrp(void);
pid_t getpid(void);
```

Job control also needs `<signal.h>` and `<termios.h>` (not in `unistd.h`).

## Other headers you will also include a lot

- `#include <sys/wait.h>`: `waitpid()`, `WIFEXITED`, `WEXITSTATUS`, ...
- `#include <fcntl.h>`: `open()`, `O_CREAT`, `O_TRUNC`, `O_APPEND`, `O_CLOEXEC`, ...
- `#include <errno.h>`: `errno`
- `#include <string.h>`: `strerror(errno)`
- `#include <signal.h>`: `sigaction()`, signals like `SIGCHLD`, `SIGINT`, ...

## Fast way to learn a syscall: use manpages

- `man 2 fork`
- `man 2 execve`
- `man 2 waitpid`
- `man 2 pipe`
- `man 2 dup2`

When writing a shell, the details that matter are usually in:
- **RETURN VALUE**
- **ERRORS** (`errno`)
- **NOTES** (inheritance rules across `fork()`/`exec()`)
