# `exec*()` in a shell: replace the child process image

In a shell, `exec*()` is how the child process turns into the program you requested.

Core rule:
- **If `exec*()` succeeds, it does not return.**
- If it returns, it returned `-1` and set `errno`.

## Headers and prototypes (syntax first)

```c
#include <unistd.h>

int execve(const char *pathname, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);
```

`execve()` is the low-level syscall.
`execvp()` is convenient early in your shell project because it does **PATH lookup**.

Related notes:
- [Fork And Wait.md](Fork%20And%20Wait.md)
- [dup() and dup2().md](dup%28%29%20and%20dup2%28%29.md)

## argv and envp “shapes” (this is where bugs happen)

### `argv[]`

- Type: `char *const argv[]` (an array of pointers)
- Must be **NULL-terminated**
- Convention: `argv[0]` is the program name (programs rely on it)

Example shape:

```c
char *argv[] = {"ls", "-l", NULL};
```

### `envp[]` (when you use `execve`)

- Array of `"KEY=VALUE"` strings
- Also NULL-terminated

Most shells pass the current environment:

```c
extern char **environ;
/* execve(path, argv, environ); */
```

## The variants (quick map)

You’ll mostly care about these:

|Call|When you use it in a shell|
|---|---|
|`execvp(file, argv)`|Early project: run commands using PATH (like `ls`)|
|`execve(path, argv, envp)`|Later: after you implement PATH search yourself, or you already have a full path|

Other variants (`execl`, `execv`, `execle`, …) are just convenience wrappers.

## Minimal child-side exec pattern (no shell loop here)

```c
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

static void child_exec_or_die(char *argv[]) {
    execvp(argv[0], argv);
    /* If we got here, exec failed */
    perror("execvp");

    /* Common shell exit codes (roughly bash-like): */
    if (errno == ENOENT) {
        _exit(127); /* command not found */
    }
    if (errno == EACCES) {
        _exit(126); /* found but not executable / permission denied */
    }
    _exit(1);
}
```

Notes:
- `perror()` prints a helpful message that includes `strerror(errno)`.
- Use `_exit()` in the child after failure.

## What `exec` does *not* reset (shell pitfall)

### File descriptors

Open file descriptors stay open across `exec` unless marked close-on-exec.

Shell implications:
- If you create pipes, you must close the unused ends in every process.
- Consider `O_CLOEXEC` or `FD_CLOEXEC` for “internal” FDs that must not leak into children.

### Signal dispositions

The child inherits signal handlers set by the parent shell.
Real shells typically reset some signals to defaults in the child before `exec`.
You can postpone this until your basic `fork+exec+wait` works.

## Exercises

1. Make a small program that takes `argv` from `main(int argc, char **argv)` and `execvp(argv[1], &argv[1])`.
2. Force an error (run a missing command) and confirm you return `127`.
3. Run a command with a redirected stdout using `dup2()` before `execvp()` (see next note).

Next: [dup() and dup2().md](dup%28%29%20and%20dup2%28%29.md)

