# `dup()` / `dup2()` = the core of redirection

In a shell, redirection is mostly:

1. `open()` a file
2. `dup2()` it onto `STDIN_FILENO` / `STDOUT_FILENO` / `STDERR_FILENO`
3. `close()` the original fd
4. `exec*()`

## Headers and prototypes

```c
#include <unistd.h>

int dup(int oldfd);
int dup2(int oldfd, int newfd);
```

Return conventions:
- On success: returns the new fd (`dup`) or `newfd` (`dup2`)
- On error: returns `-1` and sets `errno`

## Standard fds (every shell hard-codes these numbers)

|fd|name|
|---:|---|
|0|stdin|
|1|stdout|
|2|stderr|

## `dup()` (main use in shells: backup/restore)

Template:

```c
int saved = dup(STDOUT_FILENO);
if (saved == -1) {
    /* handle error */
}
```

Why this matters:
- Builtins often run in the **parent** process.
- If you implement `cd` in the parent and support `cd > out.txt`, you need to temporarily redirect stdout and then restore it.

## `dup2()` (main use: remap fd numbers)

Core rule:
- After `dup2(oldfd, newfd)`, anything that writes to `newfd` now goes to the same underlying open file as `oldfd`.

### `>` redirect stdout to file (truncate)

```c
#include <fcntl.h>

int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
if (fd == -1) {
    /* handle error */
}
if (dup2(fd, STDOUT_FILENO) == -1) {
    /* handle error */
}
close(fd);
```

### `>>` redirect stdout to file (append)

```c
int fd = open("out.txt", O_WRONLY | O_CREAT | O_APPEND, 0666);
if (fd == -1) {
    /* handle error */
}
dup2(fd, STDOUT_FILENO);
close(fd);
```

### `<` redirect stdin from file

```c
int fd = open("in.txt", O_RDONLY);
if (fd == -1) {
    /* handle error */
}
dup2(fd, STDIN_FILENO);
close(fd);
```

### `2>` redirect stderr

```c
int fd = open("err.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
if (fd == -1) {
    /* handle error */
}
dup2(fd, STDERR_FILENO);
close(fd);
```

## Close discipline (shell pitfall)

After a successful `dup2(fd, STDOUT_FILENO)` you should `close(fd)`.

Reason:
- You don’t want the child to keep extra descriptors open.
- Extra open pipe ends prevent EOF (see [Pipes (deep dive ).md](Pipes%20%28deep%20dive%20%29.md)).

## Close-on-exec (advanced, but useful)

If you create internal fds that must not leak into child programs, use:
- `O_CLOEXEC` in `open()`/`pipe2()` (Linux)
- or `fcntl(fd, F_SETFD, FD_CLOEXEC)`

You can postpone this until your pipeline/redirection logic works.

## Exercises

1. Write `int redirect_stdout(const char *path, int append)` that does `open + dup2 + close`.
2. Write a builtin-like function that prints something, but supports `> file` by backing up/restoring stdout using `dup()`.
3. Combine with `execvp()` in a child to implement `cmd > out.txt`.

Next: [Pipes.md](Pipes.md)