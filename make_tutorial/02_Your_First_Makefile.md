# 2. Your First Makefile

Let's create our first `Makefile` to compile a simple "Hello, World!" program.

## The C Code

First, let's create a directory `hello_world` and a file `main.c` inside it with the following content:

```c
#include <stdio.h>

int main() {
    printf("Hello, Make!\n");
    return 0;
}
```

You can find this file in the `hello_world` directory.

## The Makefile

Now, let's create a file named `Makefile` in the `hello_world` directory with the following content:

```makefile
hello: main.c
	gcc -o hello main.c
```

**Important**: Remember that the command line (`gcc...`) must be indented with a **tab**, not spaces.

This `Makefile` has one rule:

*   The target is `hello`.
*   The prerequisite is `main.c`.
*   The command to build the target is `gcc -o hello main.c`.

This rule tells `make` that to build `hello`, it needs `main.c`, and the command to do so is `gcc -o hello main.c`.

## Running `make`

Now, open your terminal, navigate to the `hello_world` directory, and run the `make` command:

```bash
$ make
gcc -o hello main.c
```

`make` will find the `Makefile` in the current directory and execute the command to build the `hello` target. You should now have an executable file named `hello` in your directory.

You can run it like this:

```bash
$ ./hello
Hello, Make!
```

## How `make` works

When you run `make`, it looks for the first target in the `Makefile` (in this case, `hello`) and tries to build it.

1.  `make` checks if the target `hello` exists.
2.  If it doesn't exist, `make` checks if its prerequisite, `main.c`, exists.
3.  Since `main.c` exists, `make` runs the command `gcc -o hello main.c` to create `hello`.

Now, run `make` again:

```bash
$ make
make: 'hello' is up to date.
```

This time, `make` sees that `hello` already exists and is newer than its prerequisite `main.c`, so it doesn't do anything.

Now, modify `main.c` (e.g., change the message) and run `make` again. `make` will see that `main.c` has been modified and will recompile the program.

In the next section, we'll see how to use `make` to compile a project with multiple source files.
Next : [[03_Compiling_Multiple_Files]]

