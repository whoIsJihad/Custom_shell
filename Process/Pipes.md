# Pipes: how `|` is built (from first principles)

## What a pipe really is: in-kernel FIFO buffer

When you run `ls | grep txt`, you're connecting the **stdout of `ls`** to the **stdin of `grep`**. A pipe is the mechanism that makes this work. At the kernel level, a pipe is not a file, not a socket, not a shared-memory region—it's a special **in-kernel circular buffer** that two processes can communicate through.

### The kernel pipe structure

When you call `pipe(pipefd)`, the kernel creates a single kernel-internal data structure that looks something like this:

```
struct pipe {
    char buffer[65536];          /* 64KB circular FIFO buffer */
    int head, tail;              /* read and write pointers */
    int nreaders, nwriters;      /* reference counts */
    wait_queue_t *reader_queue;  /* processes sleeping on read */
    wait_queue_t *writer_queue;  /* processes sleeping on write */
};
```

The kernel also creates two entries in the **system-wide open file table**:

1. **OFT Entry A (read end)**: marked "read-only", points to the pipe.
2. **OFT Entry B (write end)**: marked "write-only", points to the same pipe.

Then it fills your user-space array:

```c
int pipefd[2];
pipe(pipefd);  /* pipefd[0] is an FD pointing to OFT Entry A (read) */
               /* pipefd[1] is an FD pointing to OFT Entry B (write) */
```

### Why reference counts matter (critical for EOF)

The kernel tracks how many file descriptors, across the *entire system*, can read from and write to this pipe:

- **`nreaders`**: incremented for each FD opened for reading, decremented when closed.
- **`nwriters`**: incremented for each FD opened for writing, decremented when closed.

These counts are the secret to EOF. When all writers have closed their write ends (`nwriters == 0`), any further `read()` on the pipe returns 0 (end-of-file). This signals the reader that no more data will ever arrive.

If any writer is still holding the write end open (even if not writing), `nwriters > 0`, and the reader cannot know if more data is coming. So `read()` blocks indefinitely, waiting for data.

### After fork: shared FDs in both processes

When the parent forks and creates a child, the child's FD table is a **copy of the parent's array**, but each FD points to the *same OFT entry*. The kernel notices this and **increments the reference counts**:

```c
int pipefd[2];
pipe(pipefd);        /* nwriters = 1, nreaders = 1 */

pid_t pid = fork();
/* Now:
   nwriters = 2 (parent and child both hold write end)
   nreaders = 2 (parent and child both hold read end)
*/
```

This is why **you must close unused ends in each process**. If one process closes its read end and the other closes its write end, the counts drop back to 1-1, and the system works correctly. If a process forgets to close, the counts stay elevated, preventing EOF.

### The write-then-read sequence

When the parent creates a pipe and forks two children (one to write, one to read):

1. **Left child** (writer):
   ```c
   close(pipefd[0]);     /* Close read end; nreaders becomes 1 */
   write(pipefd[1], msg, len);   /* Write to pipe */
   close(pipefd[1]);     /* Close write end; nwriters becomes 1 */
   ```

2. **Right child** (reader):
   ```c
   close(pipefd[1]);     /* Close write end; nwriters becomes 1 */
   read(pipefd[0], buf, sizeof(buf));  /* Read from pipe */
   ```

3. **Parent**:
   ```c
   close(pipefd[0]);     /* nreaders becomes 1 */
   close(pipefd[1]);     /* nwriters becomes 1 */
   waitpid(left, NULL, 0);
   waitpid(right, NULL, 0);
   ```

Now the system state is stable:
- **One reader** (right child) with FD 0.
- **One writer** (left child) with FD 1.
- Parent holds nothing.

When the left child closes its write end, `nwriters` becomes 0. The right child's `read()` sees EOF and knows the conversation is over.

### What happens when you forget to close?

Suppose the parent **forgets** to close the write end:

```c
pid_t left = fork();
if (left == 0) {
    close(pipefd[0]);
    write(pipefd[1], msg, len);
    close(pipefd[1]);
    exit(0);
}

pid_t right = fork();
if (right == 0) {
    close(pipefd[1]);
    read(pipefd[0], buf, sizeof(buf));
    exit(0);
}

/* Parent forgets to close the write end */
/* close(pipefd[0]); */
/* close(pipefd[1]);  <-- FORGOT THIS */

waitpid(left, NULL, 0);
waitpid(right, NULL, 0);
```

Sequence:
1. Left child writes its message and exits, closing FD 1.
2. Right child's `read()` gets the message.
3. Right child loops again and calls `read()` for more data.
4. The kernel checks: buffer is empty, `nwriters == 1` (parent still holds it).
5. **Kernel blocks the right child**, waiting for more data.
6. But the parent is not writing; it's just sitting around.
7. **Deadlock**: right child waits forever, parent waits for right child to finish.

This is the classic "forgot to close" bug. It's so common that it's the #1 shell mistake.

### Practical pattern: N-command pipeline

To build a pipeline of N commands (`cmd1 | cmd2 | ... | cmdN`), the pattern is:

```c
void run_pipeline(char **cmds[], int n_cmds) {
    int prev_read_fd = -1;  /* FD of previous write end, or -1 if none */

    for (int i = 0; i < n_cmds; i++) {
        int current_pipe[2];
        
        /* Create a pipe for this stage (except the last stage). */
        if (i < n_cmds - 1) {
            if (pipe(current_pipe) == -1) {
                perror("pipe");
                return;
            }
        }

        pid_t pid = fork();
        if (pid == 0) {
            /* Child: run the i-th command. */

            /* Redirect stdin from the previous pipe (if not first command). */
            if (i > 0) {
                dup2(prev_read_fd, STDIN_FILENO);
                close(prev_read_fd);
            }

            /* Redirect stdout to the next pipe (if not last command). */
            if (i < n_cmds - 1) {
                dup2(current_pipe[1], STDOUT_FILENO);
                close(current_pipe[0]);
                close(current_pipe[1]);
            }

            /* Execute the command. */
            execvp(cmds[i][0], cmds[i]);
            _exit(127);
        }

        /* Parent: close the previous pipe's read end (we already connected it to the child). */
        if (i > 0) {
            close(prev_read_fd);
        }

        /* Close the write end in the parent; only the current child holds it. */
        if (i < n_cmds - 1) {
            close(current_pipe[1]);
            prev_read_fd = current_pipe[0];
        }
    }

    /* Parent: wait for all children. */
    for (int i = 0; i < n_cmds; i++) {
        int status;
        waitpid(-1, &status, 0);
    }
}
```

This pattern is complex, so study it step by step:

1. For each command stage, create a pipe (except for the last, which outputs to the terminal).
2. Fork a child to run the command.
3. In the child, redirect stdin from the previous pipe (if not first), stdout to the next pipe (if not last), and close everything you don't need.
4. In the parent, close file descriptors that the child now owns, and track the current pipe's read end for the next iteration.

The key: **every FD gets closed at least once in every process**. No leaks, no reference count bugs.

### Bidirectional pipes and process groups (advanced)

For job control, you'll need to handle process groups. If the shell wants to send a signal like SIGINT (Ctrl-C) to a whole pipeline, it needs to:

1. Create each command as a **child process group**.
2. On signal, send to the process group, not individual processes.

This is beyond the basics, but it's in your shell guide under "Process Synchronization" and "Signal Handling".

## Exercises

1. Write a simple two-stage pipeline runner. Hard-code `ls -la | grep root`.
2. Extend it to three stages: `ls | grep root | wc -l`.
3. **Test the "forgot to close" bug**: comment out a `close()` call and observe the hang.
4. Combine pipes with redirection: `cat file.txt | grep pattern > output.txt`.

## Key insights

1. **Pipes are in-kernel FIFO buffers** with reference-counted endpoints.
2. **EOF depends on reference counts**. When all writers close, readers get `0`.
3. **Both parent and child must close unused ends**. This is where most bugs happen.
4. **Pipes are unidirectional**. Data flows one way.
5. **After fork, both processes refer to the same pipe kernel structure**. The reference counts make it work.
