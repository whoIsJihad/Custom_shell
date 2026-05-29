# 3. Compiling Multiple Files

Most C projects are composed of multiple source files. In this section, we'll learn how to write a `Makefile` to manage a project with multiple files.

## The C Code

Let's consider a project with three files: `main.c`, `hello.c`, and `hello.h`. You can find these files in the `multi_file_project` directory.

`main.c`:
```c
#include <stdio.h>
#include "hello.h"

int main() {
    say_hello();
    return 0;
}
```

`hello.c`:
```c
#include <stdio.h>
#include "hello.h"

void say_hello() {
    printf("Hello from multiple files!\n");
}
```

`hello.h`:
```c
#ifndef HELLO_H
#define HELLO_H

void say_hello();

#endif
```

To compile this project manually, you would run the following commands:

```bash
$ gcc -c main.c
$ gcc -c hello.c
$ gcc -o my_program main.o hello.o
```

This is already getting tedious. Let's see how we can automate this with `make`.

## The Makefile

Here's a `Makefile` for our multi-file project:

```makefile
my_program: main.o hello.o
	gcc -o my_program main.o hello.o

main.o: main.c hello.h
	gcc -c main.c

hello.o: hello.c hello.h
	gcc -c hello.c
```

Let's break down this `Makefile`:

*   The first rule, which is the default, tells `make` that to build `my_program`, it needs `main.o` and `hello.o`.
*   The second rule tells `make` that to build `main.o`, it needs `main.c` and `hello.h`.
*   The third rule tells `make` that to build `hello.o`, it needs `hello.c` and `hello.h`.

Now, when you run `make` in the `multi_file_project` directory:

```bash
$ make
gcc -c main.c
gcc -c hello.c
gcc -o my_program main.o hello.o
```

`make` automatically determines the correct order to build the files.

## How it Works

1.  `make` wants to build the first target, `my_program`.
2.  `my_program` depends on `main.o` and `hello.o`.
3.  To build `main.o`, `make` looks for a rule for `main.o`. It finds `main.o: main.c hello.h`.
4.  `make` checks if `main.o` is older than `main.c` or `hello.h`. If it is, it runs `gcc -c main.c`.
5.  To build `hello.o`, `make` finds the rule `hello.o: hello.c hello.h` and runs `gcc -c hello.c`.
6.  Now that `main.o` and `hello.o` are up to date, `make` can build `my_program` by running `gcc -o my_program main.o hello.o`.

If you modify `hello.c` and run `make` again, it will only recompile `hello.c` and then link the object files. This is the power of `make`!

This `Makefile` is already much better than compiling manually, but it can still be improved. In the next section, we'll learn how to use variables to make our `Makefile`s more flexible and easier to read.
