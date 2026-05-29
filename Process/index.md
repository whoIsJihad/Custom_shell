# Process management for a custom C shell (index)

This folder is a focused set of notes for writing **your own shell in C**, specifically the **process-management** parts: spawning, `exec`, waiting/reaping, file descriptors, redirection, and pipelines.

You will not find a full shell implementation here.

**Note**: This folder contains both deep, narrative explanations and archived shallow versions (marked `(old)`). Start with the main notes below. The `(old)` versions are kept for reference if you want to compare or use condensed versions.
Instead, each note gives:
- the syscall prototypes + return conventions (syntax first)
- minimal C patterns you can adapt
- common pitfalls that break shells
- small exercises so you write your own project code

## How to use this directory

### Recommended reading order (core)
1. [unistd.h.md](unistd.h.md)
2. [Fork And Wait.md](Fork%20And%20Wait.md)
3. [Exec.md](Exec.md)
4. [dup() and dup2().md](dup%28%29%20and%20dup2%28%29.md)
5. [Pipes.md](Pipes.md)
6. [Pipes (deep dive ).md](Pipes%20%28deep%20dive%20%29.md) (optional but very useful)

### What you should be able to build after reading (process side)
- **Run one external command** (foreground): `fork()` + `execvp()` + `waitpid()`
- **Return correct exit status** from child to parent
- **Avoid zombies**: reap children (blocking and non-blocking)
- **Redirection**: `open()` + `dup2()` + `close()`
- **Pipelines**: `pipe()` + `fork()` + `dup2()` + close discipline

### Suggested milestones (no spoon-feeding)
- Milestone A: `spawn_one(argv)`
  - Parent waits, prints exit code.
- Milestone B: background execution + `SIGCHLD` reaping *or* periodic `waitpid(WNOHANG)`.
- Milestone C: `<`, `>`, `>>`, `2>` redirections.
- Milestone D: pipelines of N commands (`cmd1 | cmd2 | ... | cmdN`).

## Minimal “shell-grade” rules to keep in mind

- **Child code path must be small:** set up FDs, then `exec*()`; on error call `_exit(code)`.
- **Never forget to close pipe ends:** leaving an extra write end open prevents EOF.
- **Handle `EINTR`:** `waitpid()`/`read()` can be interrupted by signals.
- **In a real shell, builtins run in the parent:** that affects redirection strategy (see dup/dup2 note).

## Exercises (pick 1–2 per note)

1. Write a helper `int wait_for_pid(pid_t pid, int *exit_code)` using `waitpid()` and `WIFEXITED/WEXITSTATUS/WIFSIGNALED`.
2. Write `void reap_zombies_nonblocking(void)` that loops `waitpid(-1, &st, WNOHANG)`.
3. Implement a function that runs `argv` with stdout redirected to a file (use `dup2`).
4. Implement a two-command pipeline runner (then generalize to N commands).

## Reference-only material

- [C code for mini Shell.md](C%20code%20for%20mini%20Shell.md) contains a full shell example from elsewhere.
  - Treat it as **concept reference**, not copy/paste.
  - Pay attention to **licensing** and avoid reusing code verbatim in your own project.

## If you want to go beyond (job control)

Job control (Ctrl-C, Ctrl-Z, foreground/background process groups) is “shell process management++”.
Your workspace also has a structured guide here:
- [../Shell creation guide/10_Signal_Handling.md](../Shell%20creation%20guide/10_Signal_Handling.md)
- [../Shell creation guide/05_Process_Management.md](../Shell%20creation%20guide/05_Process_Management.md)
- [../Shell creation guide/07_Process_Synchronization.md](../Shell%20creation%20guide/07_Process_Synchronization.md)
