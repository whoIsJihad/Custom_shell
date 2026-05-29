# `fork()` and `waitpid()`: process creation and reaping

## What `fork()` actually does (the kernel's perspective)

When you call `fork()`, the kernel performs one of the most expensive operations in a Unix system: it **duplicates a running process**. To understand why this matters for your shell, you need to know what's being duplicated and why.

### Process memory layout (before fork)

A process occupies several distinct memory regions:

- **Text segment**: the executable code. This is read-only and often shared across multiple instances of the same program.
- **Data segment**: global and static variables initialized with non-zero values.
- **BSS (Block Started by Symbol)**: uninitialized globals and statics, always zero-filled.
- **Heap**: dynamically allocated memory (malloc, etc.), growing upward.
- **Stack**: local variables, function parameters, return addresses; grows downward.

Additionally, the kernel maintains:
- **File descriptor table**: a small array (per-process) where each entry is a pointer to an open file description in the kernel's system-wide file table.
- **Process control block (PCB)**: kernel structures holding the PID, registers, signal handlers, etc.

### What fork actually copies

When `fork()` returns, the kernel has created a **new process with its own address space and PCB**, but here's the critical optimization: it doesn't immediately copy all the memory. Instead, it uses **copy-on-write (COW)**:

1. **Text** stays shared (it's read-only anyway).
2. **Data and BSS** are marked as "read-only" in both parent and child. If either process tries to *write*, the kernel intercepts the page fault, makes a private copy at that moment, and lets the write proceed.
3. **Heap and stack** are similarly marked for COW.
4. **File descriptor table** is shallow-copied immediately: the child gets its own FD array, but each FD in that array points to the same kernel open file description as the parent's.

This makes `fork()` fast, even though conceptually it's creating a duplicate process.

### The fork return value and process branching

After the kernel creates the child and both processes are running, they both execute the instruction immediately after the `fork()` call. The *only* difference between them at that moment is the return value:

```c
pid_t pid = fork();
if (pid == 0) {
    /* This code runs in the CHILD. */
    /* The child's fork() call returned 0. */
} else if (pid > 0) {
    /* This code runs in the PARENT. */
    /* The parent's fork() call returned the child's PID (>0). */
} else {
    /* pid == -1: fork() failed. No child was created. */
    /* Only the parent continues. */
}
```

From the kernel's standpoint, `fork()` is a **magical function that returns twice**: once in the parent (with the child PID), and once in the child (with 0). This asymmetry is the entire point: it lets a single piece of code branch into two different execution paths.

### File descriptors are shared references

This is critical for your shell's pipe and redirection logic. When a child is created, its file descriptor table is a **copy of the parent's array**, but each entry in that array still refers to the same **underlying kernel open file description**. This means:

- If the parent has `fd 3` open to `file.txt`, the child also has `fd 3` open to the same `file.txt`.
- If the parent reads from `fd 3`, advancing the file offset to byte 1000, then the child's read from `fd 3` will start from byte 1000 (they share the offset).
- If the parent closes `fd 3`, the child's `fd 3` is unaffected (each process has its own FD array).

This shared-reference pattern is why **you must close unused pipe ends** in each process. If the child doesn't close a write end of a pipe that the parent is supposed to read from, the kernel's reference count for that write end is still 1 (the child holds it), preventing the reader from ever seeing EOF.

## Process termination and the zombie state

When a child process executes `exit()` or `_exit()`, or when it receives a signal that terminates it (like SIGKILL), the process:

1. Releases most of its resources (memory, file descriptors, etc.).
2. Tells the kernel "I'm done. Here's my exit status."
3. **Does not disappear immediately.**

Instead, the kernel transitions the child into the **zombie state**. In this state:

- The process no longer occupies significant memory (it has been cleaned up).
- But the kernel still maintains a small process table entry, holding the process's PID and exit status.
- The child is effectively invisible to the user (it doesn't appear in `ps` with a status, or appears as `<defunct>`), but the table slot remains taken.

Why does the kernel do this? Because the **parent process owns the right to read the child's exit status**. The exit status is not automatically discarded; it's preserved so the parent can query it later. This is a design choice: in Unix, process relationships matter.

A zombie remains in the process table until:

1. The parent calls `wait()` or `waitpid()` and collects the exit status, **OR**
2. The parent itself exits (then the kernel reassigns the child to PID 1, `init`, which reaps it automatically).

### Shells and zombies

In a foreground command (`ls`), the parent shell immediately calls `waitpid()`, collecting the status right away. No zombie period.

In a background command (`ls &`), the parent shell does **not** wait immediately. The child exits and becomes a zombie. The parent shell must later reap it—typically via a `SIGCHLD` signal handler that calls `waitpid(..., WNOHANG)` in a loop, or via explicit background job management. If the shell never reaps background children, the process table gradually fills with zombies.

## `waitpid()`: collecting exit status from a child

The `waitpid()` syscall is the mechanism by which the parent **retrieves the child's exit status and cleans up the process table entry**.

### Prototype and basic behavior

Headers and prototypes (syntax first)

```c
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t fork(void);
pid_t waitpid(pid_t pid, int *status, int options);
```

## `fork()` return values (the only branching rule)

```c
pid_t pid = fork();
if (pid == 0) {
    /* child */
} else if (pid > 0) {
    /* parent (pid is child PID) */
} else {
    /* error */
}
```

- Return type: `pid_t`
- Returns: `0` (child), `>0` (parent), `-1` (error)

## What gets inherited by the child (shell-relevant)

- **PID changes** (child has a new PID), but starts executing right after `fork()`.
- **File descriptors are inherited**: the child gets copies of the parent’s FD table.
  - They still refer to the same underlying kernel open-file objects.
  - That’s why *closing unused pipe ends* matters.

## Why zombies happen (and why shells must care)

When a child exits, the kernel keeps its exit status until the parent collects it with `wait()`/`waitpid()`.
Until then, the child is a **zombie**.

Shell implication:
- foreground jobs: wait immediately
- background jobs: you still need to reap them later

## `waitpid()` (preferred over `wait()` in shells)

### Return values

- `>0`: a child PID that changed state
- `0`: only possible with `WNOHANG` (no child changed state)
- `-1`: error (`errno` tells you why)

### Decoding status (template)

```c
#include <sys/wait.h>

static int status_to_exit_code(int status) {
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status); /* common shell convention */
    }
    return 1; /* fallback */
}
```

Return type and “shape”:
- Input: `int status` (opaque bitfield from kernel)
- Output: `int exit_code` you can store/print

### Foreground wait pattern (handles `EINTR`)

```c
#include <errno.h>

int status;
for (;;) {
    pid_t r = waitpid(child_pid, &status, 0);
    if (r == -1 && errno == EINTR) {
        continue; /* interrupted by a signal; retry */
    }
    if (r == -1) {
        /* real error */
        break;
    }
    break; /* child finished */
}
```

### Reaping background children (non-blocking)

```c
int status;
for (;;) {
    pid_t r = waitpid(-1, &status, WNOHANG);
    if (r > 0) {
        /* one child reaped; loop again */
        continue;
    }
    if (r == 0) {
        break; /* no more exited children right now */
    }
    /* r == -1: either no children (ECHILD) or real error */
    break;
}
```

## Child exit: `exit()` vs `_exit()` (pitfall)

In the **child**, after a failed `exec*()`, prefer `_exit(code)`.

- `exit()` runs stdio flush and `atexit()` handlers inherited from the parent.
  That can duplicate buffered output.
- `_exit()` terminates immediately.

## Options you’ll need later (job control)

If you implement Ctrl-Z / stop / continue:
- `waitpid(pid, &st, WUNTRACED | WCONTINUED)`
- check `WIFSTOPPED(st)`, `WSTOPSIG(st)`, `WIFCONTINUED(st)`

You can defer this until after basic pipelines + redirection work.

## Exercises (don’t copy/paste a whole shell)

1. Implement `int wait_foreground(pid_t pid)` that returns the computed “shell exit code”.
2. Implement `void reap_all(void)` using `waitpid(-1, ..., WNOHANG)`.
3. Make a tiny program that forks 3 children and prove you can reap all of them.

Next: [Exec.md](Exec.md)
