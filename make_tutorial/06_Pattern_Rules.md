# 6. Pattern Rules

In our current `Makefile`, the rules for `main.o` and `hello.o` are very similar. We can use a **pattern rule** to write a single rule that can build any `.o` file from a `.c` file.

## What are Pattern Rules?

A pattern rule is a rule that uses the `%` character as a wildcard. The `%` matches any non-empty string. A pattern rule looks like this:

```makefile
%.o: %.c
    command
```

This rule tells `make` how to create a file ending in `.o` from a file with the same name ending in `.c`.

## Rewriting our `Makefile` with a Pattern Rule

Let's rewrite the `Makefile` for our `multi_file_project` using a pattern rule:

```makefile
CC = gcc
CFLAGS = -Wall -g
TARGET = my_program
OBJS = main.o hello.o
DEPS = hello.h

$(TARGET): $(OBJS)
    $(CC) -o $@ $^

%.o: %.c $(DEPS)
    $(CC) $(CFLAGS) -c $< -o $@
```

Let's break down the new `Makefile`:

*   We've added a `DEPS` variable to hold the list of header files that our object files depend on.
*   We've replaced the separate rules for `main.o` and `hello.o` with a single pattern rule: `%.o: %.c $(DEPS)`.

This pattern rule tells `make`:

*   To create any file ending in `.o` (e.g., `main.o`), it needs a file with the same name ending in `.c` (e.g., `main.c`) and all the files in `$(DEPS)`.
*   The command to do this is `$(CC) $(CFLAGS) -c $< -o $@`.
    *   `$<` is the name of the first prerequisite (the `.c` file).
    *   `$@` is the name of the target (the `.o` file).

Now, when `make` needs to build `main.o`, it will use this pattern rule and run the command `gcc -Wall -g -c main.c -o main.o`.

Our `Makefile` is now much more scalable. If we add a new `.c` file to our project, we just need to add the corresponding `.o` file to the `OBJS` list, and `make` will know how to build it.

In the next section, we'll learn about **phony targets**, which are targets that are not actual files.
Next : [[07_Phony_Targets]]