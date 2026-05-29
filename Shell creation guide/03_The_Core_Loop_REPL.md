# Step 3: The Core Loop (REPL)

The heart of any shell is the Read-Eval-Print Loop (REPL). This step deconstructs the REPL model and discusses how to initialize and terminate the shell.

## Deconstructing the REPL Model

The REPL is a continuous, four-phase cycle that defines the shell's interactive lifetime:

1.  **Read**: The shell blocks and waits for the user to enter a command at the prompt. It must read an entire line of text from standard input. A key challenge in C is reading a line of arbitrary length. The recommended solution is to use the POSIX `getline()` function, which automatically handles dynamic memory allocation.
2.  **Eval(uate)**: This is the core of the shell's logic. Once a line of input is successfully read, the shell must evaluate it. This involves:
    *   Parsing the command string into tokens.
    *   Interpreting special characters for redirection or piping.
    *   Determining if the command is a built-in or an external program.
    *   Executing the command.
3.  **Print**: After the command has been evaluated and executed, any output it generated will have been displayed on the screen (standard output), along with any potential error messages (standard error).
4.  **Loop**: The shell returns to the beginning of the cycle, prints a new prompt to signal its readiness, and waits for the next user command.

## Initializing the Environment

Before entering the main REPL, a shell should perform an initialization phase. For a basic shell, this involves:

*   **Initializing data structures**: Any data structures that the shell will use should be initialized.
*   **Determining interactive mode**: Check if the shell is running in an interactive mode (connected to a terminal).
*   **Setting up signal handlers**: Set up initial signal handlers to ensure the shell responds correctly to interruptions from the outset.

## Terminating the Shell

The REPL is not truly infinite; it must have a clean exit path. Termination can be triggered in two primary ways:

1.  **Explicit Command**: The user enters a built-in `exit` command. The evaluation phase recognizes this command and sets a flag to terminate the loop.
2.  **End-of-File (EOF)**: The user signals the end of input, typically by pressing `Ctrl+D` on an empty line. The `getline()` function will return `-1`, indicating EOF, which the loop must detect to terminate.

Upon deciding to terminate, the shell must perform cleanup. This is a critical step to ensure it is a well-behaved program. The primary task is to free any and all dynamically allocated memory to prevent memory leaks. After cleanup, the shell exits, returning a status code to its parent process.

In the next step, we'll focus on the "Eval" phase and discuss how to parse the user's input.
