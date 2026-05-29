# Step 5: Process Management

Once the command is parsed, the shell needs to execute it. This step explains how to use the `fork()` and `exec()` system calls to create new processes and run external commands.

## The `fork()` System Call: Creating a New Process

The fundamental mechanism for creating new processes in Unix and Linux is the `fork()` system call. Its behavior is unique and powerful: when a process (the "parent") calls `fork()`, the kernel creates a new process (the "child") that is an almost exact duplicate of the parent. This duplication includes a copy of the parent's virtual address space (its code, data, and stack), its open file descriptors, and its environment variables.

The parent and child processes then continue execution concurrently from the instruction immediately following the `fork()` call.

The most critical aspect of `fork()` is its return value, which allows the program to differentiate between the two otherwise identical processes:

*   In the **child process**, `fork()` returns `0`.
*   In the **parent process**, `fork()` returns the Process ID (PID) of the newly created child. The PID is a unique positive integer.
*   If the process creation fails (e.g., due to resource limits), `fork()` returns `-1` to the parent, and no child process is created.

This divergent return value is the linchpin of process management. By checking if the return value is zero, positive, or negative, the code can follow different execution paths for the parent and the child, allowing them to perform different tasks.

## The `exec()` Family: Transforming a Process

Creating a duplicate process with `fork()` is only half the story. To run a different program, the child process must undergo a metamorphosis using a function from the `exec()` family.

The `exec()` system calls replace the current process's entire memory image—its code, data, and stack segments—with a new program loaded from an executable file on disk. This transformation is total; if the `exec()` call is successful, the original program code is gone forever, and execution begins at the entry point of the new program. Consequently, a successful `exec()` call never returns to the calling code. It only returns (with a value of -1) if an error occurs, such as the specified program not being found.

For building a shell, the `execvp()` variant is particularly suitable for two reasons:

1.  The **`v`** in `execvp` stands for "vector," meaning it accepts the new program's arguments as a NULL-terminated array of strings (`char *argv[]`). This format perfectly matches the output of the command parser described in the previous step.
2.  The **`p`** in `execvp` stands for "path." If the filename argument does not contain a slash (`/`), `execvp` will automatically search for the executable in the list of directories specified by the `PATH` environment variable. This behavior emulates how real shells find and execute commands like `ls` or `git` without requiring the user to type their full path (e.g., `/bin/ls`).

## The Fundamental Execution Pattern

The combination of `fork()` and `execvp()` forms the canonical pattern for running external commands in a Unix shell. This "clone and replace" paradigm is the foundation of multitasking on the platform. The sequence of operations is as follows:

1.  The parent shell receives and parses a command.
2.  It calls `fork()` to create a child process.
3.  The code immediately checks the return value of `fork()`.
4.  **If in the child process (return value is 0):**
    *   The child performs any necessary setup (e.g., I/O redirection, to be discussed later).
    *   It calls `execvp()` with the parsed command and arguments.
    *   If `execvp()` fails, it is critical for the child to print an error message (e.g., "command not found") and then terminate immediately using `exit()`. Failure to exit would leave a confused clone of the shell running.
5.  **If in the parent process (return value is the child's PID):**
    *   The parent process typically calls `waitpid()` to suspend its own execution until the child process completes. This ensures that commands are executed sequentially and the prompt is only displayed after the previous command has finished.

In the next step, we'll differentiate between external commands and built-in commands, which do not use the `fork()`-`exec()` pattern.
