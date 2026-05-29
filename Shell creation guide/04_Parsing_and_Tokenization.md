# Step 4: Parsing and Tokenization

The "Eval" phase of the REPL begins with parsing. This step covers the process of transforming the raw input string into a structured format that the shell can execute.

## The Goal of Parsing

The raw string of characters a user types into the prompt is a high-level expression of intent. The operating system's execution machinery, however, deals in structured arguments and system calls. The fundamental purpose of the parsing stage is to bridge this gap.

Parsing is the process of lexical and syntactic analysis that transforms the unstructured character string into a structured, hierarchical data representation that the shell's execution engine can unambiguously interpret and act upon. The output of this process must clearly delineate the executable command, its associated flags and arguments, and any special operators that modify the command's execution environment.

## Tokenization: The First Step

The initial phase of parsing is **tokenization** (or word splitting), which involves breaking the input string into a sequence of meaningful substrings called **tokens**. In the context of a simple shell, the primary delimiters that separate tokens are whitespace characters: spaces and tabs.

For example, the command `ls -l /home` would be tokenized into three distinct strings: `"ls"`, `"-l"`, and `"/home"`.

A common and straightforward tool for this task in the C standard library is the `strtok()` function. This function is designed to be called iteratively to extract successive tokens from a string.

## The Caveats of `strtok()`

While `strtok()` is convenient, it comes with critical caveats that must be understood to be used correctly:

*   **Destructive**: `strtok()` is a destructive function. It modifies the original input string by replacing the delimiter character that follows each token with a null terminator (`\0`). This is how it creates distinct, null-terminated C-strings for each token within the original buffer. If the original command line is needed for any other purpose (e.g., for a history feature), a copy must be made before passing it to `strtok()`.
*   **Not Reentrant**: The standard `strtok()` function is not reentrant or thread-safe. It uses a hidden, internal static pointer to keep track of its position within the string between calls. For a simple, single-threaded shell, this is not an immediate concern. However, it is good practice to be aware of the reentrant alternative, `strtok_r()`.

A simple `strtok()`-based parser is sufficient for basic commands but inherently struggles with more complex shell syntax, such as handling quoted arguments (e.g., treating `"hello world"` as a single token).

## Data Structure for Parsed Commands

The ultimate goal of the parsing process is to produce a data structure that is directly consumable by the `execvp()` system call, which is used to execute external programs. This system call expects the command and its arguments to be passed as a NULL-terminated array of character pointers (`char *argv[]` or `char **argv`).

Therefore, the parser's output should be a dynamically allocated array of `char*`. The structure should be as follows:

*   `argv[0]` points to the token representing the command itself (e.g., `"ls"`).
*   `argv[1]`, `argv[2]`, etc., point to the tokens for each subsequent argument (e.g., `"-l"`, `"/home"`).
*   The final element of the array must be a `NULL` pointer. This `NULL` acts as a sentinel, marking the end of the argument list for `execvp()`.

This transformation of an unstructured string into a formal `argv` array is the successful completion of the parsing stage, preparing the command for execution.

In the next step, we'll explore how to use this `argv` array to execute commands using the `fork()` and `exec()` system calls.
