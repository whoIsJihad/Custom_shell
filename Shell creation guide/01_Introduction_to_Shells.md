# Step 1: Introduction to Shells

This first step introduces the concept of a shell, its place in the operating system, and the fundamental concepts you'll need to understand before you start coding.

## What is a Command-Line Shell?

A command-line shell is a program that acts as an interface between the user and the operating system's kernel. It's a command-line interpreter that reads the user's commands and executes them. This process is often referred to as a **Read-Eval-Print Loop (REPL)**:

1.  **Read**: The shell reads the command from the user.
2.  **Eval**: The shell evaluates and executes the command.
3.  **Print**: The shell prints the output of the command to the terminal.
4.  **Loop**: The shell waits for the next command.

The primary function of a shell is to translate human-readable commands (like `ls` or `pwd`) into low-level system calls that the kernel can understand.

## The Shell's Place in the Linux Architecture

The Linux architecture can be visualized as a series of layers:

1.  **Hardware**: The physical components of the computer.
2.  **Kernel**: The core of the operating system, responsible for managing hardware and providing system calls.
3.  **Shell**: A user-space application that provides an interface to the kernel's services.
4.  **Applications**: Other user-space programs, such as text editors, web browsers, and the commands the shell executes.

It's important to understand that the shell is a user-space program, not part of the kernel. This means you can write your own shell without modifying the operating system itself.

## Types of Shells

There are many different shells, each with its own features and scripting syntax. Some of the most common shells include:

*   **Bourne Shell (`sh`)**: The original Unix shell, known for its simplicity and scripting power.
*   **C Shell (`csh`)**: Introduced features like command history and job control, with a syntax similar to the C programming language.
*   **Korn Shell (`ksh`)**: Combined features from both `sh` and `csh`.
*   **Bourne-Again Shell (`bash`)**: The most common shell on modern Linux systems, it's a POSIX-compliant shell with many advanced features.
*   **Z Shell (`zsh`)**: A modern, highly customizable shell with a focus on interactive use.

When building a new shell, you'll typically start with a subset of the core functionality defined by the POSIX standard.

## Shell vs. Terminal

It's important to distinguish between a shell and a terminal:

*   **Terminal**: The terminal (or terminal emulator) is a graphical application that provides a window for text-based input and output (e.g., `gnome-terminal`, `xterm`).
*   **Shell**: The shell is the command interpreter program that runs inside the terminal window.

## Built-in vs. External Commands

There are two types of commands that a shell can execute:

*   **External Commands**: These are separate, standalone executable programs located on the filesystem (e.g., `ls`, `grep`, `gcc`). The shell's job is to find these programs and run them in a new process.
*   **Built-in Commands**: These commands are implemented directly within the shell's code (e.g., `cd`, `exit`, `export`). They are necessary for tasks that need to modify the shell's internal state, such as changing the current working directory.

In the next step, we'll look at how to structure the C project for our shell.
## Next : [[02_Project_Structure]]