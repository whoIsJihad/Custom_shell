# C Strings Practice

This directory contains small C programs for practicing C string handling and buffer processing.

## `declaration.c`

### What it does
- Reads a line of text from standard input using `fgets` into a fixed buffer: `char line[100];`
- Iterates through the buffer until the null terminator `\0` is reached
- Replaces any newline character `\n` with a space character `' '` while scanning
- Prints each non-null character and its index position

### Key concepts practiced
- Fixed-size character array declaration
- `fgets` input reading with a buffer size limit
- Null-terminated string iteration
- Detecting and handling the newline character inserted by `fgets`
- Printing characters and their positions

### Notes
- The program stops scanning when it reaches the end of the string (`'\0'`).
- If the input line contains a newline, it is replaced with a space, but the program does not print the newline itself.
- The buffer length is 100, so inputs longer than 99 characters may be truncated by `fgets`.

## `strtok.c`

### What it does
- Reads a line of text from standard input using `fgets` into a mutable buffer: `char line[100];`
- Uses `strtok` to split the input into tokens separated by whitespace characters: spaces, tabs, and newlines
- Prints one token per line

### Key concepts practiced
- `strtok` usage for tokenizing a C string in place
- Mutable input buffers, since `strtok` writes `\0` into the buffer
- Iterating through tokens with `strtok(NULL, delim)`
- Simple shell-style parsing of a command line

### Notes
- `strtok` modifies the input string, so the original line is split into separate tokens inside the same buffer.
- Use `char *token = strtok(line, delim);` for the first token and `token = strtok(NULL, delim);` for each subsequent token.
- This example is a good step toward building a shell parser before adding `fork()` and `exec()`.
