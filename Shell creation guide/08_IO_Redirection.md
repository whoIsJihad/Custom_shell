## Prev : [[07_Process_Synchronization]]
# Step 8: I/O Redirection

I/O redirection is a powerful feature that allows you to control the input and output of commands. This step explains the concepts behind I/O redirection and how to implement it using file descriptors and the `dup2()` system call.

## The Unix Philosophy: "Everything is a File"

A core design principle of Unix-like operating systems is the abstraction that treats disparate I/O resources—such as files on a disk, pipes, network sockets, and terminal devices—as if they were all files. This elegant simplification means that a small, consistent set of system calls (`open`, `read`, `write`, `close`) can be used to interact with a wide variety of resources. The handle that a process uses to refer to one of these open "files" is a **file descriptor**: a simple, non-negative integer.

## The Three Standard Streams

By convention, every process inherits three pre-opened file descriptors from its parent when it is created:

*   **File Descriptor 0: Standard Input (`stdin`)**. This is the default source of input for a program.
*   **File Descriptor 1: Standard Output (`stdout`)**. This is the default destination for normal output.
*   **File Descriptor 2: Standard Error (`stderr`)**. This is the default destination for error messages.

For an interactive shell, all three of these streams are initially connected to the user's terminal, allowing for keyboard input and screen output.

## The `dup2()` System Call: The Key to Redirection

The mechanism that enables I/O redirection is the `dup2(int oldfd, int newfd)` system call. Its function is to duplicate a file descriptor, but with a crucial twist: it makes `newfd` a copy of `oldfd`, but forces the new descriptor's number to be `newfd`. The process is atomic:

1.  If `newfd` is already an open file descriptor, the kernel silently closes it.
2.  The kernel then makes `newfd` a copy of `oldfd`. Both `oldfd` and `newfd` now refer to the same underlying open file entry in the kernel, sharing the same file offset and status flags.

This ability to overwrite a specific file descriptor (like `1` for `stdout`) with a copy of another (like one pointing to a disk file) is the key to redirection.

## Implementing Redirection (`>` and `<`)

Redirection is an act of deception orchestrated by the shell. The programs being executed (`ls`, `grep`, etc.) are typically written to be oblivious to redirection; they simply read from `stdin` (fd 0) and write to `stdout` (fd 1). The shell leverages the window of opportunity between `fork()` and `execvp()` to manipulate the child's file descriptor table, tricking the new program into reading from or writing to a file instead of the terminal.

The sequence for **output redirection** (e.g., `ls -l > output.txt`) is as follows:

1.  The parent shell parses the command and identifies the `>` operator and the target filename `output.txt`.
2.  The parent shell calls `fork()`.
3.  **Inside the child process:**
    a. The child opens the file `output.txt` for writing using the `open()` system call. This returns a new file descriptor, let's say `fd = 3`.
    b. The child then calls `dup2(3, 1)`. This is the critical step. The kernel closes the original `stdout` (fd 1, which pointed to the terminal) and makes fd 1 a duplicate of fd 3. Now, any write operation directed at file descriptor 1 will be sent to `output.txt`.
    c. The original file descriptor 3 is now redundant and should be closed with `close(3)`.
    d. Finally, the child calls `execvp("ls", ...)` The `ls` program starts, completely unaware of this manipulation. It performs its work and writes its output to its standard output (file descriptor 1), which the kernel now routes to `output.txt`.

**Input redirection** (`<`) follows a similar pattern, but involves opening a file for reading and using `dup2(fd, 0)` to overwrite `stdin`.

In the next step, we'll extend this concept to implement pipes, allowing the output of one command to be used as the input for another.

## Next : [[09_Pipes]]