## Prev : [[08_IO_Redirection]]
# Step 9: Pipes

Pipes extend the concept of redirection from files to processes, allowing the `stdout` of one command to be connected directly to the `stdin` of another. This step explains how to implement pipes.

## Implementing Pipes (`|`)

A pipe is a unidirectional, in-memory buffer managed by the kernel, behaving like an anonymous temporary file. The `pipe(int pipefd[2])` system call creates a pipe and provides two file descriptors in the integer array `pipefd`:

*   `pipefd[0]` is the **read end** of the pipe.
*   `pipefd[1]` is the **write end** of the pipe.

The complex sequence for a command like `cmd1 | cmd2` is a masterful coordination of two child processes and a pipe:

1.  The parent shell calls `pipe(pipefd)` to create the pipe.
2.  The shell calls `fork()` to create the first child (for `cmd1`).
3.  **In Child 1 (`cmd1`):**
    a. It will write to the pipe, so it does not need the read end. It calls `close(pipefd[0])`.
    b. It redirects its `stdout` to the write end of the pipe by calling `dup2(pipefd[1], 1)`.
    c. It closes the now-redundant original write end, `close(pipefd[1])`.
    d. It calls `execvp()` to run `cmd1`. All of `cmd1`'s output will now flow into the pipe.
4.  The shell calls `fork()` again to create the second child (for `cmd2`).
5.  **In Child 2 (`cmd2`):**
    a. It will read from the pipe, so it does not need the write end. It calls `close(pipefd[1])`.
    b. It redirects its `stdin` from the read end of the pipe by calling `dup2(pipefd[0], 0)`.
    c. It closes the now-redundant original read end, `close(pipefd[0])`.
    d. It calls `execvp()` to run `cmd2`. `cmd2` will read its input from the pipe, receiving `cmd1`'s output.
6.  **In the Parent Shell:**
    a. The parent does not read from or write to the pipe itself. It must close both ends, `close(pipefd[0])` and `close(pipefd[1])`. This is crucial; the read end of the pipe will not signal EOF to the reading process (`cmd2`) until all write ends have been closed.
    b. The parent calls `waitpid()` twice to reap both children.

In the final step, we'll cover how to handle interruptions and signals, making the shell more robust and interactive.

## Next : [[10_Signal_Handling]]