# Synchronization In Linux Kernel

- What is concurrency?
  - The ability to handle multiple outstanding task/process with the illusion or reality of simultaneity.
  - Single Core Environment (fake parallelism):
    - Concurrency is achieved via a process called context-switch i.e., at a particular time period, only a single task gets executed.
  - Multiple Core Environment (True parallelism):
    - Multiple processes executing simultaneously on multiple processors/CPU's.

## 1. Background of Multiprocessing

### 1.1. Multiprocessing systems

- Multiprocessing systems
  - Each CPU has its own OS.
  - The simplest possible way to organize a multiprocessor OS:
    - Statically divide memory into as many partitions as there are CPUs and give each CPU its own private memory and its own private copy of the OS.
    - One obvious optimization is to allow all the CPUs to share the OS code and make private copies of only the data.

    CPU 1           CPU 2           CPU 3           CPU 4           Memory
|Private OS|    |Private OS|    |Private OS|    |Private OS|    |Data 1|Data 2|
    ||              ||              ||               ||         |Data 3|Data 4|
    ||              ||              ||               ||         |   OS code   |
    ||              ||              ||               ||                ||
    ||              ||              ||               ||                ||       | IO |
____||______________||______________||_______________||________________||_________||_
_______________________________________BUS___________________________________________

- Problems with this approach:
  - When a process makes a system call, the system call is caught and handled on its own CPU using the data structures in the OS's table.
  - Since each OS has its own tables, it also has its own set of processes that is schedules by itself. There is no sharing of processes. As a consequence, it can happen that CPU 1 is idle while CPU 2 is loaded with work.
  - No sharing of pages. It can happen that CPU 1 has pages to share while CPU 2 is paging continuously. There is no way for CPU 2 to borrow some pages from CPU 1 since the memory allocation is fixed.
  - If the OS maintains a buffer cache of recently used disk blocks, each OS does this independently of the other ones. Thus it can happen that a certain disk block is present and dirty in multiple buffer caches at the same time, leading to inconsistent results.

### 1.2. A master-slave multiprocessor model

- One copy of OS and its tables are present on CPU 1 and not on any of the others.
- All system calls are redirected to CPU 2 for processing there.
- CPU 1 may also run user processes if there is  CPU time left over.
- This model is called master-slave since CPU 1 is the master and all the others is slave.

    CPU 1        CPU 2/Slave     CPU 3/Slave     CPU 4/Slave            Memory
|Master |       |User process|  |User process|  |User process|      |User process |
|runs OS|           ||              ||               ||             |             |
    ||              ||              ||               ||             |   OS code   |
    ||              ||              ||               ||                    ||
    ||              ||              ||               ||                    ||       | IO |
____||______________||______________||_______________||____________________||_________||__
_______________________________________BUS________________________________________________

- Whenever a user process comes, Master CPU will assign to one of the slave offices and the master if free, it can also run the usual processes.

- The master-slave model solves most of the problems of the first model:
  - There is a single data structure (e.g.,one list or a set of priorities lists) that keeps track of ready processes. When a CPU goes IDLE, it asks the OS for a process to run anf it is assigned one. Thus it can never happen that one CPU is IDLE while another is overloaded.
  - Similarly, pages can be allocated among all the processes dynamically and there is only one buffer cache, so inconsistencies never occur.

- Problem:
  - The problem with this model is that with many CPUs, the master will become a bottleneck.
  - After all, it must handle all system calls from all CPUs.
  - If, say, 10% of all time is spent handling system calls, then 10 CPUs will pretty much saturate the master, and with 20 CPUs it will be completely overloaded.
  - Thus, this model is simple and workable for small multiprocessors, but for large ones it fails.

### 1.3. Symmetric Multiprocessor (SMP)

- One copy of the OS is in memory, but any CPU can run it.

    CPU 1           CPU 2           CPU 3            CPU 4              Memory
|Shared OS/  |   |Shared OS/  |  |Shared OS/  |  |Shared OS/  |      |             |
|User process|   |User process|  |User process|  |User process|      |             |
    ||              ||              ||               ||              |   OS code   |
    ||              ||              ||               ||                    ||
    ||              ||              ||               ||                    ||       | IO |
____||______________||______________||_______________||____________________||_________||__
_______________________________________BUS________________________________________________

- Advantage: eliminates the master CPU bottleneck, since there is no master.
- Disadvantage: Image two CPUs simultaneously picking the same process to run or claiming the same free memory page.
  - The simplest way around these problems is to associate a mutex (i.e., lock) with the OS, making the whole system one big critical region.
  - When aCPU wants to run OS code, it must first acquire the mutex.
    - If the mutex is locked, it just waits.
    - In this way, any CPU can run the OS, but __only one at a time__.
  - With 20 CPUs, there will be long queues of CPUs waiting to get in.
  - Fortunately, it is easy to improve. Many parts of the OS are independent of one another.
  - For example, there is no problem with one CPU running the scheduler while another CPU is handling a file system call and a third one is processing a page fault.
  - This observation leads to splitting the OS up into independent critical regions that do no interact with one another. Each critical region is protected by its own mutex, so only one CPU at a time can execute it.

## 2. Preemption and context switch in Linux Kernel

- What is preemption?
  - `Preemption` means forcefully taking away of the processor from one process and allocating it to another process.
  - Switch from one running task/process is known as `context switch`.

- How it is implemented on Linux Kernel?
  - In the Linux Kernel, the scheduler is called after each timer interrupt (that is, quite a few times per second).It determines what process to run next based on a variety of factors, including priority, time already run, etc.

- Different between `preemption` and `context switch`?
  - `preemption`: Firing of timer interrupt is preempting the current running process and running the interrupt service routine of timer interrupt.
  - `context switch`: What happens when the kernel alters the state of the processor (the registers, mode, and stack) between one process or thread's context and another.
    - [context_switch()](https://github.com/torvalds/linux/blob/master/kernel/sched/core.c) function is called in the kernel.

### 2.1. Preemption in kernel space and user space

- User space:
  - Under Linux, US programs have always been preemptible: the kernel interrupts US programs to switch to other threads, using the regular clock tick.
  - So the kernel doesn't wait for US programs to explicitly release the processor. This means that an infinite loop in an US program CANNOT block the system.

- Kernel space:
  - In computer operating system design, kernel preemption is a property possessed by some kernels (the cores of operating systems), in which the CPU can be interrupted in the middle of executing kernel code and _assigned other tasks_ (from which it later returns to finish its kernel tasks).

  - Until 2.6 kernels, the kernel itself was not preemptible, as soon as one thread has entered the kernel, it could not be preempted to execute an other thread.
    - The processor could be used to execute another thread when a system call was terminated, or when the current thread explicitly asked the scheduler to run another thread using the `schedule()` function.
    - This means that an infinite loop in the kernel code blocked the entire system.
  - However, this absence of preemption in the kernel caused several problems with regard to latency and scalability.
  - So, kernel preemption has been introduced in 2.6 kernels, and one can enable or disable it using the `CONFIG_PREEMPT` option.
    - An infinite loop in the code can no longer block the entire system.

- The __main benefit__ of kernel preemption is that it solves two issues that would otherwise be problematic for monolithic kernels, in which the kernel consists of one large binary. Without kernel preemption, two major issues exist for monolithic and hybrid kernels:

  - A device driver can enter an infinite loop or other unrecoverable state, crashing the whole system.
  - Some drivers and system calls on monolithic kernels can be slow to execute, and cannot return control of the processor to the scheduler or other program until they complete execution.

### 2.2. When can kernel preemption happen

- When returning to kernel-space from an interrupt handler.
- When kernel code becomes preemptible again.
- If a task in the kernel explicitly calls `schedule()`.
- If a task in the kernel blocks (which results in a call to `schedule()`).

### 2.3. Example

- First example, while process A executes an exception handler (necessarily in Kernel Mode), a higher priority process B becomes runnable.
  - This could happen, if an IRQ occurs and the corresponding handler awakens process B.
  - As the kernel is preemptive, a forced process switch replace process A with B.
  - The exception handler is left unfinished and will be resumed only when scheduler selects again process A for execution.

- More example, consider a process that executes an exception handler and whose time quantum expires. As the kernel is preemptive, the process may be replaced immediately.

- Motivation for making the kernel preemptive
  - reduce the dispatch latency of the User Mode processes:
    - the delay between the time they become runnable and the time they actually begin running.
  - Processes performing timely scheduled tasks (such as external hw controllers, environmental monitors, movie players, and so on) really benefit from kernel preemption, because it reduces the risk of being delayed by another process running in kernel mode.

## 3. Reentrancy

- Kernel functions are executed following a request that may be issued in two possible ways:
  - A Process executing in user mode causes an exception, for instance by executing an int 0x80 assembly language instruction.
  - An external device sends a signal to a Programmable Interrupt Controller by using an IRQ line, and the corresponding interrupt is enabled.

- What is a `kernel control path`?
  - A `kernel control path` denotes the sequence of instructions executed by the kernel to handle a system call, an exception, or an interrupt.

- `Reentrant` kernels
  - Linux kernel is reentrant. This means that several processes (kernel threads) may be executing in Kernel Mode at the same time.
  - On uniprocessor systems, only one process can progress, but many can be blocked in Kernel Mode when waiting for the CPU or the completion of some I/O operation.
  - Example:
    - After issuing a read to disk on behalf of a process, the kernel lets the disk controller handle it and resumes executing  other processes.
    - An interrupt notifiers the kernel when the device has satisfied the read, so the former process can resumes executing other processors.

- How do we have reentrancy?
  - Reentrancy in Linux Kernel:
    - Reentrant functions: they don't use/modify global data structures.
    - Non reentrant functions: Modify global data structures but use locking mechanism.

## 4. Synchronization Race condition and critical regions

- Implementing a reentrant kernel requires the use of synchronization.
- If a kernel control path os suspended while acting on a kernel data structure, no other kernel control path should be allowed to act on the same data structure unless it has been reset to a consistent state.

- Otherwise the interaction of the two control paths could corrupt the stored information.

- Examples:
  - Suppose a global variable V contains the number of available items of some system resource.
    Kernel Control path A                     Kernel Control path B
    |----------------------------------------------------------------|
    reads the var and value is 1
                                              reads the same var, and
                                              value is 1
                                              increments V
    increments V

  - Final value of V is 2, instead of 3 which is wrong.
- When the outcome of a computation depends on how two or more processes are scheduled, the code is incorrect. We say that there is a race condition.

- Any section of code that should be finished by each process that begins it before another process can enter it is called a `critical region`.

## 5. Causes of concurrency

1. Interrupts: An interrupt can occur asynchronously at almost any time; interrupting the currently executing code.

2. Soft-IRQs and tasklets: kernel can raise or schedule a soft-irq or tasklet al almost any time.

3. kernel preemption: because the kernel is preemptive, one task in the kernel can preempt another.

4. Sleeping and synchronization with user space: task in the kernel can sleep and thus invoke the scheduler, resulting in running of a new process.

5. Symmetrical multiprocessor: two or more processors can execute kernel code at exactly the same time.

## 6. Solutions for concurrency

- Simple solutions:
  - Kernel preemption Disabling:

    ```C

    // disabling kernel preemption.
    // critical region start.

    // do some thing ...

    // critical region end.

    // enabling kernel preemption.
    ```

    - Problem: On multiprocessor, two kernel paths running on different CPUs can concurrently access the same global data.
  - Hardware interrupt Disabling:

    ```C

    // disabling Hardware interrupt.
    // critical region start.

    // do some thing ...

    // critical region end.

    // enabling Hardware interrupt.
    ```

    - Problem:
      - If the critical region is large, interrupts can remain disabled for a relatively long time, potentially causing all hardware activities to freeze.
      - On a multiprocessor system, disabling interrupts on the local CPU is not sufficient, and other synchronization techniques must be used.

## Find out which processor is running kernel control path

- `smp_processor_id()` gives u the current processor number on which kernel is running.

## Find out maximum number of processors in kernel

```C
// using global macro.
int nr_cpu = NR_CPUS;

// Or get exactly online CPU.
int cpu = num_online_cpus();
```

## Per CPU variable

- The simplest and most efficient synchronization technique consists of declaring kernel variables as per-CPU variables.
- Basically, a per CPU variables is an array of data structures, one element per each CPU in the system.
- A CPU should not access the elements of the array corresponding to other CPU.
- IT can freely read and modify its own element without fear of race conditions, because it is the only entitled to do so.
- The elements of the per-CPU array are aligned in main memory so that each data structure falls on a different line of the hardware cache.

### per-cpu interface

- The 2.6 kernel introduced a new interface, known as per-cpu, for creating and manipulating per-CPU data.

- Creation and manipulation of per-cpu data is simplified with this new approach.
- This new interface, however, grew out of the needs for a simpler and more powerful method for manipulating per-CPU data on large symmetrical multiprocessing computers.

```C
#include <linux/percpu.h>

// DEFINE_PER_CPU(type, name);

DEFINE_PER_CPU(int, x);
```

- This creates an interface of a variable of type `int`, named `x`, for each processor on the system.

- If u need a declaration of the variable elsewhere, to avoid compile warnings, the following macro is your friend:

```C
#include <linux/percpu.h>

DECLARE_PER_CPU(int, x);
```

- You can manipulation the variables with the `get_cpu_var()` and `put_cpu_var()` routines.

- A call to `get_cpu_var()` returns an lvalue for the given variable on the current processor. It also disables preemption, which `put_cpu_var()` correspondingly enables.

### Access per cpu variable on another processor

- U can access another processor's copy of the variable with:

```C
per_cpu(variable, cpu_id);
```

- If u write code that involves processors reaching into each other's per-cpu variables, u, of course, have to implement a locking scheme that makes that access safe.

### per cpu at runtime

- dynamically allocated per-cpu variables are also possible

```C
void *alloc_percpu(type); /* Macro.*/
void *__alloc_percpu(size_t size, size_t align);
void free_percpu(const void *);
```

## Semaphore

- semaphores
  - Semaphores in Linux are sleeping locks

- what happens when the semaphores lock is unavailable?
  - The semaphore places the task onto a wait queue and puts the task to sleep.
  - The processor is then free to execute other code.

- What happens after the semaphore becomes available
  - One of the tasks on the wait queue is awakened so that it can then acquire the semaphore.

- Implementation
  - A semaphore is a single integer value combined with a pair of functions that are typically called P and V (Dutch words).
  - `P()` -> Probe-ren (test).
  - `V()` -> Verhogen (increment).
  - Entering critical section:
    - A process wishing to enter a critical section will call `P()` on the relevant semaphore.
      - If the semaphore's value is greater than zero, that value is decremented by one and the process continues.
      - If, instead, the semaphore's value is 0 (or less), the process must wait until somebody else releases the semaphore.
  - Exiting critical section:
    - Accomplished by calling `V()`.
    - This function increments the value of the semaphore and, if necessary, wakes up processes that are waiting.

### Types of semaphore

- Types of semaphore
  - Spin locks allow only one task to hold the lock at a time.
  - With semaphores, number of tasks to hold the lock at a time can be specified while initializing/declaring semaphore.
  - This value is called as usage count or simply count.
    - Count = 1 --> Binary semaphore, used for mutual exclusion.
    - Count . 1 --> Counting semaphore.

### Can I use counting semaphores in the critical sections?

- I think NOT, because only one process has to be in the critical section.

- Counting semaphores are not used to enforce mutual exclusion because they enable multiple threads of execution in the critical region at once.

- instead, they are used to enforce limits in certain code.

- They are not used much in the kernel.

### spin lock vs semaphore

- Which one to choose for critical region: spin lock vs semaphore?
  - Sleep: semaphore is the only option.
  - Lock hold time: semaphores are good for longer lock hold times, spin locks are useful when the lock hold time is small.
  - Scheduling latency: As semaphores do not disable kernel preemption, scheduling latency is better when compared to spin locks.
