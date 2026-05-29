# 7. Phony Targets

So far, all our targets have been actual files that `make` creates. However, sometimes we want to create targets that are not files. These are called **phony targets**.

## What are Phony Targets?

A phony target is a target that is not a real file. It's just a name for a recipe of commands. `make` doesn't check if a file with that name exists or if it's up to date. It always runs the commands for a phony target when you ask it to.

The most common use for phony targets is to create short names for common tasks, such as cleaning up build files or building the entire project.

## The `clean` Target

Let's add a `clean` target to our `Makefile` to remove the executable and all the object files:

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

clean:
    rm -f $(TARGET) $(OBJS)
```

Now, you can run `make clean` to remove all the generated files:

```bash
$ make clean
rm -f my_program main.o hello.o
```

However, there's a potential problem here. What if you have a file named `clean` in your directory? In that case, `make` will see that the file `clean` exists and has no prerequisites, so it will think that the `clean` target is up to date and will not run the `rm` command.

To fix this, we need to tell `make` that `clean` is a phony target. We do this by declaring it as a prerequisite of the special `.PHONY` target:

```makefile
.PHONY: clean
```

## Our Final `Makefile`

Here's our final `Makefile` for the `multi_file_project`, with a `clean` phony target and an `all` phony target (which is a common convention for the default target):

```makefile
CC = gcc
CFLAGS = -Wall -g
TARGET = my_program
OBJS = main.o hello.o
DEPS = hello.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
    $(CC) -o $@ $^

%.o: %.c $(DEPS)
    $(CC) $(CFLAGS) -c $< -o $@

clean:
    rm -f $(TARGET) $(OBJS)
```

Now, `make all` (or just `make`) will build the project, and `make clean` will clean it up, regardless of whether there are files named `all` or `clean` in the directory.

In the final section, we'll briefly touch on some more advanced `make` topics.
