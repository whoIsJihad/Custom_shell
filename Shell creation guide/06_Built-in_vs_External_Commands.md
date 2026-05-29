# Step 6: Built-in vs. External Commands

After parsing a command, the shell must decide how to execute it. This step explains the difference between built-in and external commands and why built-in commands are necessary.

## Defining Built-in and External Commands

The command execution logic within a shell follows a crucial bifurcation. After parsing a user's command, the shell must decide upon one of two distinct execution pathways. This decision is based on whether the command is external or built-in.

*   **External Commands** are standalone executable programs that reside in the filesystem, typically in directories like `/bin`, `/usr/bin`, or `/usr/local/bin`. Commands like `ls`, `grep`, `gcc`, and `find` fall into this category. The shell executes these by employing the `fork-exec` pattern detailed in the previous section: it creates a new child process, and that child process transforms itself into the desired program.
*   **Built-in Commands** are not separate programs. Their logic is implemented directly as functions within the shell's own source code. When a user invokes a built-in command, the shell executes it by simply calling the corresponding internal function. No new process is created via `fork()`, and no external program is loaded via `exec()`.

This distinction means the core of the shell's execution engine is a decision tree. After parsing the command, the first logical step is to check if the command name matches one of the known built-ins. If it does, the shell calls the relevant internal C function. If not, it proceeds with the `fork-exec` mechanism for external commands.

## The Rationale for Built-ins: Modifying the Shell's State

The necessity for built-in commands is not an arbitrary design choice but a direct and fundamental consequence of the process isolation model inherent to Unix-like operating systems. When `fork()` creates a child process, the child receives a *copy* of the parent's memory space and environment, not a reference to it. This isolation is a critical feature for system stability and security; it prevents a child process from maliciously or accidentally corrupting its parent's state.

The canonical example that illustrates this principle is the `cd` (change directory) command. Every process maintains its own "current working directory" (CWD), which is an attribute of the process's state managed by the kernel. If `cd` were an external program (e.g., `/bin/cd`), executing it would follow this sequence:

1.  The parent shell would `fork()` a child process.
2.  The child process would `exec()` the `/bin/cd` program.
3.  The `/bin/cd` program, running inside the child process, would successfully call the `chdir()` system call to change *its own* current working directory.
4.  The `/bin/cd` program would then finish its task and exit immediately.
5.  The parent shell, which had been waiting, would resume execution, its own current working directory having been completely unaffected by the child's actions.

The change would be lost the moment the child process terminated. Therefore, any command that must fundamentally alter the state of the shell itself—its working directory, its environment variables (`export`), its aliases (`alias`), or its very existence (`exit`)—**must** be implemented as a built-in command. By implementing `cd` as a function within the shell, the call to the `chdir()` system call occurs directly within the shell's process, correctly modifying its own CWD.

## Other Built-ins and Performance

Beyond the necessity of state modification, there is a secondary reason for implementing certain simple commands as built-ins: **performance**. The process of forking and executing a new program involves significant overhead, including memory allocation, copying page tables, and loading a new binary from disk. For very simple and frequently used commands (e.g., `echo`, `pwd`), implementing them as built-ins avoids this overhead, resulting in noticeably faster execution.

In the next step, we'll discuss how the parent shell waits for its child processes to complete, a concept known as process synchronization.

## Next  : [[07_Process_Synchronization]] 