# Step 2: Project Structure

## Prev : [[01_Introduction_to_Shells]]
A well-organized project structure is essential for managing the complexity of a systems-level application like a shell. This step outlines a recommended project structure and explains the role of a `Makefile`.

## Structuring a C Project

A modular approach to project structure promotes clarity, maintainability, and scalability. Here's a recommended structure for your shell project:

*   **`src/`**: This directory will contain all your C source code files (`.c` files). Logic should be separated into modules based on functionality. For example:
    *   `main.c`: The main entry point and the REPL.
    *   `parser.c`: Logic for parsing commands.
    *   `execute.c`: Logic for executing commands.
    *   `builtins.c`: Implementation of built-in commands.
*   **`include/`**: This directory will contain all your header files (`.h` files). Each header file should define the public API for its corresponding source module. This allows modules to interact through well-defined interfaces.
*   **`bin/`**: This directory will store the final compiled executable binary. Keeping build artifacts separate from source code is a clean practice.
*   **`Makefile`**: A crucial file at the root of the project that contains the instructions for the `make` utility to build the entire project from its source files.

This separation of concerns makes the codebase easier to navigate and facilitates independent development and testing of different components.

## The Role of the `Makefile`

The `Makefile` is the blueprint for your project's build process. It automates the tasks of compiling source code into object files and linking those object files into a final executable.

A `Makefile` is composed of **rules**, each of which defines how to create a specific file, known as a **target**. The core components of a rule are:

*   **Target**: The file to be created, such as an object file (`main.o`) or the final executable (`myshell`).
*   **Prerequisites (Dependencies)**: A list of files that the target depends on. For an object file, the prerequisites are its source (`.c`) and any header (`.h`) files it includes. For an executable, the prerequisites are all the object files that need to be linked together.
*   **Commands**: A sequence of shell commands (e.g., `gcc`, `ld`) that are executed to create the target from its prerequisites. **Note**: Command lines in a `Makefile` must be indented with a tab character, not spaces.

The primary advantage of using `make` is its ability to perform intelligent, incremental builds. `make` will only rebuild a target if it doesn't exist or if any of its prerequisites have been modified more recently than the target itself. This saves a significant amount of time by avoiding unnecessary recompilation of unchanged code.

In the next step, we'll dive into the heart of the shell: the Read-Eval-Print Loop (REPL).
## next : [[03_The_Core_Loop_REPL]]