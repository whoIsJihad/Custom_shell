# `dup2()`: the mechanism behind redirection

## File descriptor tables: two levels of indirection

To understand `dup2()`, you need to understand the **two-level indirection** that Unix uses for file descriptors.

### Level 1: Per-process file descriptor table

When you open a file or create a pipe, the kernel returns an integer: the **file descriptor**. This integer is an **index into a per-process array** in the kernel:

```
Process A's FD table (kernel memory, one per process):
FD 0 -> (pointer) -> OFT entry X (stdin)
FD 1 -> (pointer) -> OFT entry Y (stdout)
FD 2 -> (pointer) -> OFT entry Z (stderr)
FD 3 -> (pointer) -> OFT entry M (some file)
```

The indices 0, 1, 2 are conventional (stdin, stdout, stderr). When you call `open()`, the kernel finds the next available index and fills it in.

### Level 2: System-wide open file table

At the kernel level, there's a **global open file table (OFT)** shared across all processes. Each entry in the OFT represents a single open file:

```
System-wide OFT (kernel memory, shared):
Entry X -> {file: /dev/tty, offset: 1234, flags: O_RDONLY}
Entry Y -> {file: /dev/tty, offset: 5678, flags: O_WRONLY}
Entry Z -> {file: /dev/tty, offset: 5678, flags: O_WRONLY}
Entry M -> {file: data.txt, offset: 0, flags: O_WRONLY | O_CREAT}
```

Each entry holds:
- The actual file inode.
- The current read/write offset (where the next `read()` or `write()` will occur).
- Flags (read-only, write-only, append, etc.).

### The two-level design matters

This indirection is why `dup2()` is powerful: **multiple FDs in the same process (or different processes) can point to the same OFT entry**. When you `dup2(fd1, fd2)`, you make fd2 point to the same OFT entry as fd1. Both FDs now refer to the same file, and they **share the same file offset**.

When the parent forks, the child's FD table is a shallow copy: the child gets its own array, but each entry points to the same OFT entry as the parent's. This is how `fork()` "inherits" open files.

## What `dup2()` does

### Prototype and semantics

```c
#include <unistd.h>

int dup2(int oldfd, int newfd);
```

`dup2(oldfd, newfd)` makes **`newfd` point to the same OFT entry as `oldfd`**. Specifically:

1. If `newfd` is already open, close it (and decrement the OFT entry's reference count).
2. Set `FD_table[newfd]` to point to the same OFT entry as `FD_table[oldfd]`.
3. Increment the OFT entry's reference count (now two processes, or two FDs in one process, refer to it).

After the call, `oldfd` and `newfd` refer to the same underlying file. They share:
- The same file inode (same file on disk).
- The same current offset (if one reads/writes, the other's offset advances too).
- The same flags (O_APPEND, etc.).

If `oldfd` is invalid, `dup2()` fails and returns `-1`.

### Example: redirecting stdout

Here's the classic pattern: make stdout go to a file instead of the terminal.

```c
#include <fcntl.h>
#include <unistd.h>

int main() {
    /* Open a file for writing. */
    int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    /* Make FD 1 (stdout) point to the file. */
    if (dup2(fd, STDOUT_FILENO) == -1) {
        perror("dup2");
        return 1;
    }

    /* Close the original fd. */
    close(fd);

    /* Now printf() writes to the file, not the terminal. */
    printf("This goes to out.txt\n");

    return 0;
}
```

What happened:
1. `open()` returned fd 3 (the first available FD in this process).
2. `dup2(3, 1)` made FD 1 point to the same OFT entry as FD 3.
3. FD 1 and FD 3 now refer to the same file, with the same offset.
4. `close(3)` closed the original fd. FD 1 still points to the file.
5. `printf()` writes to FD 1, which goes to the file.

### Buffering gotcha: when printf buffering matters

If you redirect stdout before calling `printf()`, there's no problem: the output goes to the file. But if you redirect *after* some data is already in stdout's buffer, the buffered data might get lost or go to the wrong place.

In a shell, this rarely happens because the parent does not print to stdout before forking and redirecting in the child. But in a more complex application, you might need to flush:

```c
fflush(stdout);        /* Flush any pending output. */
dup2(fd, STDOUT_FILENO);
```

### Stdin redirection

Reading from a file instead of the terminal is the mirror image:

```c
int fd = open("input.txt", O_RDONLY);
if (fd == -1) {
    perror("open");
    return 1;
}

dup2(fd, STDIN_FILENO);  /* Make FD 0 point to the file. */
close(fd);

/* Now scanf() reads from the file. */
int x;
scanf("%d", &x);  /* Reads from input.txt */
```

### Combining redirection and pipes

In your shell, you often need to combine these. For example, `cmd1 < input.txt | cmd2 > output.txt`:

```c
/* In the child for cmd1: */
int input_fd = open("input.txt", O_RDONLY);
dup2(input_fd, STDIN_FILENO);
close(input_fd);
dup2(pipe_read, STDOUT_FILENO);  /* Also redirect stdout to the pipe. */
close(pipe_read);
close(pipe_write);
execvp(argv[0], argv);

/* In the child for cmd2: */
dup2(pipe_read, STDIN_FILENO);  /* Read from the pipe. */
close(pipe_read);
int output_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
dup2(output_fd, STDOUT_FILENO);
close(output_fd);
execvp(argv[0], argv);
```

The key pattern: **always close** the original FD after `dup2()`, so you don't leak reference counts.

## Using dup2() in a shell (typical patterns)

### Pattern 1: output redirection (`>`)

```c
int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
if (fd == -1) {
    perror("open");
    _exit(1);
}
dup2(fd, STDOUT_FILENO);
close(fd);
execvp(argv[0], argv);
_exit(127);
```

### Pattern 2: append redirection (`>>`)

```c
int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
if (fd == -1) {
    perror("open");
    _exit(1);
}
dup2(fd, STDOUT_FILENO);
close(fd);
execvp(argv[0], argv);
_exit(127);
```

### Pattern 3: input redirection (`<`)

```c
int fd = open(filename, O_RDONLY);
if (fd == -1) {
    perror("open");
    _exit(1);
}
dup2(fd, STDIN_FILENO);
close(fd);
execvp(argv[0], argv);
_exit(127);
```

### Pattern 4: stderr redirection (`2>`)

```c
int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
if (fd == -1) {
    perror("open");
    _exit(1);
}
dup2(fd, STDERR_FILENO);  /* Note: FD 2, not 1. */
close(fd);
execvp(argv[0], argv);
_exit(127);
```

## dup2() in builtins (for advanced shells)

Some commands are **builtins**: they run in the parent shell process, not in a child. Examples: `cd`, `set`, `export`.

If a builtin needs to support redirection (e.g., `echo hello > /tmp/out.txt`), you must:

1. **Backup** the original FD using `dup()`:
   ```c
   int saved = dup(STDOUT_FILENO);
   ```

2. **Redirect** to the file:
   ```c
   int fd = open("out.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
   dup2(fd, STDOUT_FILENO);
   close(fd);
   ```

3. **Run the builtin**:
   ```c
   builtin_echo("hello");  /* Prints to the file. */
   ```

4. **Restore** the original:
   ```c
   dup2(saved, STDOUT_FILENO);
   close(saved);
   ```

This is more complex than the child process approach, but necessary if you want builtins to respect redirections.

## Why close the original FD?

After `dup2(fd, STDOUT_FILENO)`, you have two FDs (e.g., `fd 3` and `fd 1`) pointing to the same OFT entry. Closing `fd` prevents:

1. **FD leaks**: accumulating open FD numbers in your process.
2. **Resource exhaustion**: a process can only have ~1024 FDs; wasting them is bad.
3. **Confusion**: you want to know which FDs are "active" in your process.

In the shell child, after `dup2()`, you're about to `exec()`, so the FD table will be replaced anyway. But good habit: always close.

## FD_CLOEXEC: advanced, but useful

If you create "internal" file descriptors that should not leak into child processes, mark them with `FD_CLOEXEC`:

```c
int fd = open("config.txt", O_RDONLY);
fcntl(fd, F_SETFD, FD_CLOEXEC);  /* Auto-close on exec. */
```

This is useful for shell state files, logging, etc. You can defer this until after basic pipelines work.

## Exercises

1. Write a program that opens a file, uses `dup2()` to make stdout go to the file, and prints several lines. Verify the output file.
2. Extend it: implement `cmd > output.txt` by forking and using `dup2()` in the child.
3. Implement `cat < input.txt` by forking and using `dup2()` to redirect stdin.
4. Combine: `cat < input.txt > output.txt` by doing both redirections.
5. Test the "buffering" issue: print to stdout, then redirect; notice if anything goes to the terminal.

## Key insights

1. **File descriptors are indices into a per-process array**.
2. **Multiple FDs can point to the same OFT entry** (file).
3. **`dup2()` makes one FD point to another's OFT entry**.
4. **After `dup2()`, both FDs share the same offset**—reads and writes by one process affect the other's position.
5. **Close the original FD after `dup2()`** to prevent leaks.
6. **Use this for redirection in child processes** before `exec()`.
7. **For builtins, use `dup()`/`dup2()` to save/restore state** if they need redirection support.
