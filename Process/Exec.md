# `exec*()` system calls: replacing a process image with a new program

## What "execute" really means: complete memory replacement

When you call `execvp("ls", argv)`, you're asking the kernel to do something radical: **replace the running process's code, data, and heap with a completely different program**. The child process still has the same PID (the parent's `fork()` return value doesn't change), but from the moment `execvp()` succeeds, the process is running entirely different code.

### Why separate `fork()` and `exec()`?

A natural question: why not have a single syscall like `spawn("ls", argv)` that combines fork and execute? The answer is: **you need to change file descriptors between fork and exec**.

When you redirect output (`ls > out.txt`), the sequence is:

1. **`fork()`**: create a child process.
2. **`open("out.txt", ...)`**: open a file in the child.
3. **`dup2(fd, STDOUT_FILENO)`**: change FD 1 (stdout) to point to the file.
4. **`execvp("ls", argv)`**: run `ls`. Its output goes to `out.txt` instead of the terminal.

If fork and exec were merged, you'd have no chance to intervene and change FDs. So Unix designers kept them separate, giving the child process a chance to set up its world before exec.

### The exec operation: what the kernel does

When `execvp("ls", argv)` succeeds, the kernel:

1. **Loads the executable file** (`/bin/ls`) into memory. The ELF (or other) format specifies where the text, data, and BSS segments go.
2. **Replaces the entire address space** of the current process with the new program's. Old code, data, heap, and stack are discarded.
3. **Initializes the new process state**: the program's `main()` is set up on the stack, and the CPU's instruction pointer jumps to the program's entry point (usually `_start`, which calls `main`).
4. **Keeps the same PID** and most kernel state. File descriptors, signal handlers, process groups—all stay the same unless explicitly reset.
5. **Returns only on error**. If exec succeeds, it does not return. The old code is gone.

### argv and envp: the interface to the new program

When `execvp("ls", argv)` loads `/bin/ls`, the kernel needs to tell the new program what arguments it was given. The convention is:

- **`argv[0]`** is the program name (by convention, the command name, not the full path). The new program should use this for error messages like "ls: file not found".
- **`argv[1], argv[2], ...`** are the actual arguments.
- **`argv[n]` is `NULL`**, marking the end.

For example:

```c
char *argv[] = {"ls", "-l", "/tmp", NULL};
execvp("ls", argv);
```

The kernel copies this array into the new process's stack (and into the `envp` area if you provide one), where the new program can read it.

The **`envp`** array is a list of environment variable strings in `"KEY=VALUE"` format, also NULL-terminated. If you use `execvp()` (no 'e' for environment), the child inherits the parent's environment. If you use `execve()`, you can provide a custom environment.

### The variants: which one to use

You'll see several `exec*` variants in manpages. They differ in:

- **List vs. array**: `execl("path", "arg0", "arg1", NULL)` vs. `execv("path", argv)`.
- **PATH search**: `execvp()` searches `$PATH` vs. `execv()` needs a full path.
- **Environment**: `exece()` variants accept a custom environment; others inherit the parent's.
- **System call vs. wrapper**: `execve()` is the only true syscall; the others are library wrappers.

For early shells, use **`execvp()`**: it searches `$PATH` and inherits the environment. You can move to `execve()` later when you build your own PATH searcher.

```c
#include <unistd.h>

/* execvp: search PATH, inherit environment */
execvp("ls", argv);

/* execve: full path, explicit environment */
char *env[] = {"PATH=/bin:/usr/bin", NULL};
execve("/bin/ls", argv, env);
```

### Error semantics: exec fails, then what?

If `execvp()` fails (e.g., the command is not found), it returns `-1` and sets `errno`. The process is **still the child**; the original code is still running.

This is why you must exit the child if exec fails:

```c
pid_t pid = fork();
if (pid == 0) {
    /* Child process. */
    execvp(argv[0], argv);
    
    /* CRITICAL: If we reach this line, execvp() FAILED and returned.
       On success, execvp() does NOT return—the new program replaces
       this code entirely. So reaching here = error.
    */
    
    /* perror() prints the error message from errno to stderr. */
    perror("execvp");
    
    /* errno is a global variable set by syscalls on failure.
       Check what went wrong, then exit the child with a conventional code.
    */
    if (errno == ENOENT) {
        /* ENOENT = "No such file or directory" = command not found. */
        _exit(127);
    } else if (errno == EACCES) {
        /* EACCES = "Permission denied" = file exists but not executable. */
        _exit(126);
    } else {
        /* Some other error (disk error, memory error, etc). */
        _exit(1);
    }
    
    /* Note: We use _exit(), not exit().
       exit() would flush buffers and run atexit() handlers inherited from
       the parent, which we don't want. _exit() dies immediately.
    */
}
```

Why `_exit()` instead of `exit()`?

Simple: `exit()` does cleanup (closes files, calls functions registered by `atexit()`). But the child process is throwing away its parent's resources anyway. Just die immediately with `_exit()`. Cleaner.

### Common exit code conventions (for shells)

When a command finishes, it returns a number (0–255) called the **exit code**. Your shell reads this and stores it in `$?`.

The conventions are:

- **0**: command succeeded.
- **1**: command failed (generic error).
- **2**: command misused (wrong arguments).
- **126**: command found but not executable.
- **127**: command not found.
- **128 + N**: command killed by signal N (e.g., 130 = killed by SIGINT, signal 2).
- **Other**: whatever the command returned.

Your shell just collects the exit code from `waitpid()` and passes it through. Nothing special.

### Important: File descriptors pass through exec

This is the **key** to redirection working at all.

Here's a concrete example:

```c
/* Redirect output of 'ls' to a file. */
int fd = open("listing.txt", O_WRONLY | O_CREAT, 0644);
dup2(fd, 1);           /* Make stdout point to file */
close(fd);             /* Close original fd; now only FD 1 is open */

/* Now exec. */
execvp("ls", argv);

/* When 'ls' runs, it writes to stdout (FD 1).
   FD 1 is now the file, so output goes there. */
```

Run: `./myshell ls`. Output doesn't go to terminal; it goes to `listing.txt`.

That's all redirection is: change where FD 1 (or 2, or 0) points, then exec the program.

**Exec does not close file descriptors.** They pass through unchanged. This is intentional—it's how you set up the child's world before exec.

### Summary for your shell (ignore these for now)

- Signal handlers and process groups: advanced topic. Skip until job control.
- FD_CLOEXEC: advanced flag to hide internal file descriptors from children. Skip.

## A minimal fork + exec + wait example (C code)

Here's a tiny shell that runs one command:

```c
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s command [args...]\n", argv[0]);
        return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        /* Child: exec the command. */
        execvp(argv[1], &argv[1]);
        perror("execvp");
        
        /* Exit with appropriate code. */
        if (errno == ENOENT) {
            _exit(127);
        }
        _exit(126);
    }

    /* Parent: wait for child. */
    int status;
    waitpid(pid, &status, 0);
    
    /* Print exit code. */
    if (WIFEXITED(status)) {
        printf("Exit: %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("Killed by signal: %d\n", WTERMSIG(status));
    }

    return 0;
}
```

Run it:

```bash
gcc -o myshell myshell.c
./myshell ls -la
./myshell echo hello world
./myshell /bin/nonexistent  # Should print "command not found" and exit 127
```

## Key insights for your shell

1. **Exec is radical**: it replaces the entire process image. Old code, data, everything is gone.
2. **Exec does not return on success**. If it returns, it failed.
3. **argv[0] is a convention**. The new program expects it to be the command name, not the full path.
4. **File descriptors survive exec**. This is how redirection works: you `dup2()` a file onto FD 1, then `execvp()`, and the new program writes to that file.
5. **Use `execvp()` early**, then optimize to `execve()` when you've built your own PATH searcher.
6. **On exec failure, exit the child** with a conventional code (127 for not found, 126 for permission).
7. **Use `_exit()` in the child** after a failed exec, not `exit()`.
