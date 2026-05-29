# 8. Advanced Topics

This tutorial has covered the fundamentals of `make`. With what you've learned, you can write `Makefile`s for most of your C projects. However, `make` is a very powerful tool with many advanced features. This section will give you a brief introduction to some of these features.

## Functions

`make` has a set of built-in functions that you can use to manipulate text, work with files and directories, and perform other tasks.

For example, the `wildcard` function can be used to get a list of all files that match a certain pattern. The `patsubst` function can be used to perform a search and replace on a string.

Here's how you could use these functions to automatically find all the `.c` files in your directory and generate the list of `.o` files:

```makefile
SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))
```

This is much more convenient than listing all the files manually.

## Conditionals

You can use conditionals in your `Makefile` to execute different commands based on certain conditions. For example, you could have a `DEBUG` variable that, when set, compiles the code with debugging flags:

```makefile
ifeq ($(DEBUG), 1)
    CFLAGS += -g
endif
```

## Including Other `Makefile`s

You can use the `include` directive to include other `Makefile`s in your `Makefile`. This is useful for large projects where you might want to have separate `Makefile`s for different parts of the project.

```makefile
include config.mk
```

## Further Learning

This is just a glimpse of what you can do with `make`. To learn more, I highly recommend reading the [GNU Make Manual](https://www.gnu.org/software/make/manual/). It's a comprehensive resource that covers all the features of `make` in detail.

Happy making!
