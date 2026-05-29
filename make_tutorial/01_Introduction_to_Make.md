# 1. Introduction to Make

Welcome to this step-by-step tutorial on `make`! This guide will teach you how to use `make` to automate the process of building your C projects.

## What is `make`?

`make` is a build automation tool that automatically builds executable programs and libraries from source code by reading files called `Makefile`s which specify how to derive the target program. Though it is most famously used for C/C++ projects, it can be used for any project where files need to be updated when other files change.

## Why use `make`?

Imagine you have a project with several source files. When you change one file, you only need to recompile that file and then link all the object files together to create the executable. Doing this manually can be tedious and error-prone.

`make` automates this process for you. It can:

*   Keep track of which files have changed.
*   Only recompile the files that need to be recompiled.
*   Link the object files to create the final executable.
*   Perform other tasks, such as cleaning up build files or installing the program.

## Core Concepts

A `Makefile` consists of a set of **rules**. A rule has the following structure:

```makefile
target: prerequisites
    command
```

*   **Target**: The name of the file to be created. This is often an executable or an object file.
*   **Prerequisites** (or dependencies): A list of files that the target depends on. If any of the prerequisites change, the target needs to be rebuilt.
*   **Command**: A shell command that `make` executes to create the target. **Important**: The command line must start with a **tab character**, not spaces.

Here's a simple example:

```makefile
my_program: main.o utils.o
    gcc -o my_program main.o utils.o
```

In this example:

*   `my_program` is the target.
*   `main.o` and `utils.o` are the prerequisites.
*   `gcc -o my_program main.o utils.o` is the command.

This rule tells `make` that to create `my_program`, it needs `main.o` and `utils.o`. If either of those files changes, `make` will run the `gcc` command to recreate `my_program`.

In the next section, we'll create our first `Makefile` and use it to compile a simple C program.
Next : [[02_Your_First_Makefile]]