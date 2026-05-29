# 4. Using Variables

Our `Makefile` for the multi-file project works, but it has some repetition. If we want to change the compiler or add some compiler flags, we have to edit multiple lines. We can use variables to make our `Makefile`s more readable and easier to maintain.

## What are Variables?

In a `Makefile`, you can define variables to hold strings of text. By convention, variable names are in uppercase. You define a variable like this:

```makefile
VARIABLE_NAME = value
```

To use a variable, you enclose it in `$(...)`:

```makefile
$(VARIABLE_NAME)
```

## Rewriting our `Makefile` with Variables

Let's rewrite the `Makefile` for our `multi_file_project` using variables:

```makefile
CC = gcc
CFLAGS = -Wall -g
TARGET = my_program
OBJS = main.o hello.o

$(TARGET): $(OBJS)
    $(CC) -o $(TARGET) $(OBJS)

main.o: main.c hello.h
    $(CC) $(CFLAGS) -c main.c

hello.o: hello.c hello.h
    $(CC) $(CFLAGS) -c hello.c
```

Let's break down the variables we've defined:

*   `CC`: The C compiler to use (e.g., `gcc`).
*   `CFLAGS`: The compiler flags to use (e.g., `-Wall` to enable all warnings, `-g` to include debugging information).
*   `TARGET`: The name of our executable.
*   `OBJS`: The list of object files.

Now, our `Makefile` is much cleaner and easier to modify. If we want to change the compiler or add a new compiler flag, we only need to change one line.

## Benefits of Using Variables

*   **Readability**: Variables make your `Makefile` easier to read and understand.
*   **Maintainability**: You can easily change values in one place, and the changes will be reflected throughout the `Makefile`.
*   **Portability**: You can easily change the compiler or compiler flags to adapt your `Makefile` to different systems.

This `Makefile` is a big improvement, but we can still do better. In the next section, we'll learn about **automatic variables**, which can make our `Makefile`s even more concise.
