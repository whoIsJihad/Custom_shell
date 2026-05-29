# 5. Automatic Variables

`make` provides several "automatic variables" that can make your `Makefile`s even more concise and powerful. These variables are automatically set by `make` for each rule, based on the target and prerequisites of the rule.

## Common Automatic Variables

Here are some of the most common automatic variables:

*   `$@`: The name of the target.
*   `$<`: The name of the first prerequisite.
*   `$^`: The names of all prerequisites, separated by spaces.

## Rewriting our `Makefile` with Automatic Variables

Let's rewrite the `Makefile` for our `multi_file_project` using automatic variables:

```makefile
CC = gcc
CFLAGS = -Wall -g
TARGET = my_program
OBJS = main.o hello.o

$(TARGET): $(OBJS)
    $(CC) -o $@ $^

main.o: main.c hello.h
    $(CC) $(CFLAGS) -c $<

hello.o: hello.c hello.h
    $(CC) $(CFLAGS) -c $<
```

Let's see how the automatic variables are used here:

*   In the rule for `$(TARGET)`, `$@` is `my_program` and `$^` is `main.o hello.o`. So the command becomes `gcc -o my_program main.o hello.o`.
*   In the rule for `main.o`, `$@` is `main.o` and `$<` is `main.c`. So the command becomes `gcc -c main.c`.
*   In the rule for `hello.o`, `$@` is `hello.o` and `$<` is `hello.c`. So the command becomes `gcc -c hello.c`.

Our `Makefile` is now much more generic. We don't have to repeat the filenames in the commands.

This is a huge improvement, but we can still do better. Notice that the rules for `main.o` and `hello.o` are very similar. In the next section, we'll learn about **pattern rules**, which will allow us to write a single rule to compile all our `.c` files into `.o` files.
