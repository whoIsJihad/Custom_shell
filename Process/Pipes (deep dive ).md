# A kernel-level explanation of Unix pipes (why shells must `close()` correctly)

If you’re writing a shell, the main value of this deep dive is understanding:

- why forgetting to close unused pipe ends causes **hangs**
- what EOF on a pipe really means (when `read()` returns `0`)
- why `SIGPIPE` happens

Read the simpler usage template first:
- [Pipes.md](Pipes.md)

This is a step-by-step breakdown of what happens inside the kernel when you create and use a pipe. We will avoid analogies and focus on the data structures and process states.

## 1. The Core Kernel Data Structures

First, let's look at what the kernel creates. When you call `pipe()`, you're not creating a file on a disk. You are asking the kernel to create an in-memory communication object. This object consists of three main parts:

1. **The Pipe Structure (The "Inode"):** This is a kernel-space structure, like a `struct pipe` in xv6. It's the central object. It contains:
    
    - **A Data Buffer:** A fixed-size, in-memory circular buffer (e.g., 64KB in Linux, smaller in xv6). This is the "FIFO" (First-In, First-Out) part.
        
    - **Buffer Pointers/Counters:** A read pointer (`head` or `nread`) and a write pointer (`tail` or `nwrite`) to manage the circular buffer and track how much data is in it.
        
    - **Reference Counters:** This is the critical part. It has _at least_ two counts:
        
        - `read_open_count`: The number of file descriptors in the _entire system_ that can _read_ from this pipe.
            
        - `write_open_count`: The number of file descriptors that can _write_ to this pipe.
            
    - **Wait Queues:** Two lists of processes:
        
        - `reader_wait_queue`: Processes that are asleep, waiting for data to be written.
            
        - `writer_wait_queue`: Processes that are asleep, waiting for space to become available.
            
2. **The System-Wide Open File Table:** This is a global table inside the kernel containing an entry for _every_ open file in the _entire system_. A `pipe()` call creates **two** new entries in this table:
    
    - **OFT Entry A (Read End):** This entry is marked "Read-Only." It contains a pointer to the single **Pipe Structure** from step 1.
        
    - **OFT Entry B (Write End):** This entry is marked "Write-Only." It contains a pointer to the _exact same_ **Pipe Structure**.
        
3. **The Per-Process File Descriptor Table:** This is an array inside _your_ process's control block (e.g., `struct proc` in xv6). The index is the file descriptor (FD) number. The value is a pointer to an entry in the **System-Wide Open File Table**.
    

## 2. The `pipe(p)` System Call

You're in your C program and you call `int p[2]; pipe(p);`.

Here is what the kernel does, step-by-step:

1. **Allocate Pipe Structure:** The kernel allocates one `Pipe Structure` (as in 1.1).
    
    - Buffer is empty.
        
    - `read_open_count` is set to **1**.
        
    - `write_open_count` is set to **1**.
        
2. **Allocate OFT Entries:** The kernel creates `OFT Entry A` (Read) and `OFT Entry B` (Write) in the global Open File Table. Both point to the new `Pipe Structure`.
    
3. **Allocate FDs:** The kernel looks at _your process's_ File Descriptor Table (1.3). It finds two empty slots (let's say 3 and 4).
    
    - `YourProcess->fd_table[3]` is set to point to `OFT Entry A (Read End)`.
        
    - `YourProcess->fd_table[4]` is set to point to `OFT Entry B (Write End)`.
        
4. **Return:** The `pipe()` call returns, filling your user-space array `p` with `{ 3, 4 }`.
    

At this point, you have one process with two FDs (3 and 4) pointing (indirectly) to a single pipe buffer. The pipe's counts are `readers=1`, `writers=1`.

## 3. The `fork()` System Call

This is the most important moment. Your process calls `fork()`.

1. The kernel creates a new process (Child).
    
2. The kernel _copies_ the Parent's **File Descriptor Table** into the new Child process.
    
3. This is a _shallow copy_. It copies the _pointers_. It does **not** create new Open File Table entries or new Pipe Structures.
    
4. This means:
    
    - `Parent->fd_table[3]` -> `OFT Entry A`
        
    - `Child->fd_table[3]` -> `OFT Entry A`
        
    - `Parent->fd_table[4]` -> `OFT Entry B`
        
    - `Child->fd_table[4]` -> `OFT Entry B`
        
5. Because the kernel just created new references to the Open File Table entries, it follows the pointers and **increments the reference counts** on the underlying `Pipe Structure`:
    
    - `read_open_count` is incremented to **2**.
        
    - `write_open_count` is incremented to **2**.
        

Now, two processes (Parent and Child) have FDs 3 and 4, all pointing to the _same_ pipe buffer. The kernel knows this, which is why the counts are both 2.

## 4. Closing Unused Ends: The Critical Step

This is the step that causes confusion, but now you can see _why_ it's necessary. Let's say the Parent wants to _write_ (e.g., it's `ls`) and the Child wants to _read_ (e.g., it's `grep`).

1. **Parent (Writer):** It doesn't need to read. It calls `close(p[0]);` (which is `close(3)`).
    
    - **Kernel Action:** The kernel goes to Parent's FD table, finds entry 3. It follows the pointer to `OFT Entry A` and then to the `Pipe Structure`. It **decrements the `read_open_count`**. The `read_open_count` is now **1**.
        
    - The Parent's `fd_table[3]` is now marked as `[EMPTY]`.
        
2. **Child (Reader):** It doesn't need to write. It calls `close(p[1]);` (which is `close(4)`).
    
    - **Kernel Action:** The kernel goes to Child's FD table, finds entry 4. It follows the pointer to `OFT Entry B` and then to the `Pipe Structure`. It **decrements the `write_open_count`**. The `write_open_count` is now **1**.
        
    - The Child's `fd_table[4]` is now marked as `[EMPTY]`.
        

**This is the "Clean" State for IPC:**

- The Parent has `fd_table[4]` (write) open.
    
- The Child has `fd_table[3]` (read) open.
    
- The `Pipe Structure` counts are: `read_open_count = 1`, `write_open_count = 1`.
    

This is perfect. There is one reader and one writer, and the kernel knows it.

## 5. Step-by-Step Simulation: Data Flow & Blocking

Let's simulate the data flow in this clean state.

### Simulation 1: Reader Blocks

1. **Child (Reader)** calls `read(p[0], buf, 100);`
    
2. **Kernel Action:**
    
    - The kernel follows the pointers from `Child->fd_table[3]` to the `Pipe Structure`.
        
    - It checks the `data_buffer`. It's **empty**.
        
    - It checks the `write_open_count`. It's **1**.
        
    - **Kernel Decision:** "The buffer is empty, but a writer _exists_. Data might arrive. I cannot return EOF."
        
    - **Kernel Action:** The kernel adds the Child process to the `reader_wait_queue` and sets the Child's state to `BLOCKED` (or `SLEEPING`).
        
    - The `read()` system call does not return. The Child is now asleep.
        

### Simulation 2: Writer Writes and Unblocks Reader

1. **Parent (Writer)** calls `write(p[1], "hello", 5);`
    
2. **Kernel Action:**
    
    - The kernel follows the pointers from `Parent->fd_table[4]` to the `Pipe Structure`.
        
    - It checks the `data_buffer`. It has space.
        
    - **Kernel Action:** The kernel copies `"hello"` from the Parent's user-space memory into the pipe's kernel-space `data_buffer`.
        
    - It checks the `reader_wait_queue`. It's **not empty**! The Child process is waiting.
        
    - **Kernel Action:** The kernel **wakes up** the Child process (e.g., `wakeup(Child)`), moving its state from `BLOCKED` to `RUNNABLE`.
        
    - The `write()` system call returns `5` to the Parent. The Parent continues executing.
        
3. **Later:** The OS scheduler runs the `Child` process (which is now `RUNNABLE`).
    
4. **Kernel Action:** The Child's `read()` call resumes _inside the kernel_.
    
    - It now sees `"hello"` in the `data_buffer`.
        
    - It copies `"hello"` from the kernel buffer to the Child's user-space `buf`.
        
    - The `read()` system call returns `5` to the Child. The Child continues executing.
        

### Simulation 3: Writer Blocks (Pipe Full)

1. The Child (Reader) is busy or asleep and is _not_ calling `read()`.
    
2. The Parent (Writer) enters a loop and successfully calls `write()` 16 times, completely filling the 64KB `data_buffer`.
    
3. **Parent (Writer)** calls `write(p[1], "extra", 5);`
    
4. **Kernel Action:**
    
    - The kernel checks the `data_buffer`. It's **full**.
        
    - **Kernel Action:** The kernel adds the Parent process to the `writer_wait_queue` and sets the Parent's state to `BLOCKED`.
        
    - The `write()` call does not return. The Parent is now asleep.
        

## 6. Simulation Part 4: The `EOF` Signal

This is the most important concept. How does the `read()` call know when to stop? It returns `0`, which signals End-of-File (EOF). This happens **only** when:

1. The buffer is empty.
    
2. **AND** the `write_open_count` on the pipe is 0.
    

Let's simulate this. We start from our "Clean" state (`read_open_count=1`, `write_open_count=1`).

1. **Parent (Writer)** finishes writing all its data (e.g., `ls` is done).
    
2. **Parent (Writer)** calls `close(p[1]);` (its write FD, 4).
    
3. **Kernel Action:**
    
    - The kernel follows the pointers from `Parent->fd_table[4]` to the `Pipe Structure`.
        
    - It **decrements the `write_open_count`**.
        
    - The `write_open_count` on the pipe is now **0**.
        
4. **Child (Reader)** is in a loop: `while ((n = read(p[0], buf, 100)) > 0) { ... }`
    
5. **Child (Reader)** calls `read()`. The kernel reads the last bit of data (e.g., `"world"`) from the buffer. The `read()` call returns `5`. The loop continues.
    
6. **Child (Reader)** loops again, calls `read(p[0], buf, 100);`
    
7. **Kernel Action:**
    
    - The kernel checks the `data_buffer`. It is **empty**.
        
    - It checks the `write_open_count`. It is **0**.
        
    - **Kernel Decision:** "The buffer is empty, and no process in the system _ever_ will be able to write to this pipe again. This is the definition of EOF."
        
    - **Kernel Action:** The `read()` system call returns **0**.
        
8. **Child (Reader)**'s `while` loop condition `(n > 0)` becomes `(0 > 0)`, which is false. The loop terminates. The Child (e.g., `grep`) now knows the `ls` command is finished.
    

## 7. The Deadlock (Forgetting to `close()`)

Now you can see _why_ forgetting to `close()` is a disaster.

**The Mistake:** The Parent `forks()`. Parent wants to write, Child wants to read.

- Parent (Writer) _correctly_ calls `close(p[0]);` (its read end).
    
- Child (Reader) _**forgets**_ to call `close(p[1]);` (its write end).
    

**The State Before Simulation:**

- `Parent->fd_table[4]` (write) is open.
    
- `Child->fd_table[3]` (read) is open.
    
- `Child->fd_table[4]` (write) is **STILL OPEN**.
    
- **Kernel `Pipe Structure` counts:** `read_open_count = 1`, `write_open_count = 2`.
    

**The Deadlock Simulation:**

1. **Parent (Writer)** writes all its data.
    
2. **Parent (Writer)** calls `close(p[1]);` (its write FD, 4).
    
3. **Kernel Action:** The kernel decrements the `write_open_count`. The count goes from 2 to **1**.
    
4. **Child (Reader)** is in its `while ((n = read(p[0], ...)) > 0)` loop.
    
5. **Child (Reader)** calls `read()`, drains the last bit of data from the buffer.
    
6. **Child (Reader)** loops again, calls `read(p[0], buf, 100);`
    
7. **Kernel Action:**
    
    - The kernel checks the `data_buffer`. It is **empty**.
        
    - It checks the `write_open_count`. It is **1**. (The Child _itself_ is holding it open!).
        
    - **Kernel Decision:** "The buffer is empty, but a writer exists. Data _might_ arrive."
        
    - **Kernel Action:** The kernel puts the **Child Process** on the `reader_wait_queue` and sets its state to `BLOCKED`.
        
8. **DEADLOCK.** The Child is now permanently asleep. It is waiting for data from a "writer" that will never write (because it's the Child itself). It is waiting for an EOF signal (`write_open_count == 0`) that can never come, because _it_ is the process preventing the count from dropping to 0.
    

## Summary: Blocking and Signaling Rules

|System Call|Condition|Kernel Action|
|---|---|---|
|**`read(fd, ...)`**|Buffer has data.|Returns data (up to requested size).|
|**`read(fd, ...)`**|Buffer is empty, `write_open_count > 0`.|**Block** (process sleeps on `reader_wait_queue`).|
|**`read(fd, ...)`**|Buffer is empty, `write_open_count == 0`.|Returns `0` (this is the EOF signal).|
|**`write(fd, ...)`**|Buffer has space.|Writes data, wakes up any sleeping readers.|
|**`write(fd, ...)`**|Buffer is full.|**Block** (process sleeps on `writer_wait_queue`).|
|**`write(fd, ...)`**|`read_open_count == 0`.|Fails, returns `-1` (`EPIPE`), sends `SIGPIPE` signal.|