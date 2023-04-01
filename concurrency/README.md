# Synchronization In Linux Kernel

- What is concurrency?
  - The ability to handle multiple outstanding task/process with the illusion or reality of simultaneity.
  - Single Core Environment (fake parallelism):
    - Concurrency is achieved via a process called context-switch i.e., at a particular time period, only a single task gets executed.
  - Multiple Core Environment (True parallelism):
    - Multiple processes executing simultaneously on multiple processors/CPU's.

## Background of Multiprocessing

### Multiprocessing systems

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

### A master-slave multiprocessor model

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

### Symmetric Multiprocessor (SMP)

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

## Preemption and context switch in Linux Kernel

- What is preemption?
  - `Preemption` means forcefully taking away of the processor from one process and allocating it to another process.
  - Switch from one running task/process is known as `context switch`.

- How it is implemented on Linux Kernel?
  - In the Linux Kernel, the scheduler is called after each timer interrupt (that is, quite a few times per second).It determines what process to run next based on a variety of factors, including priority, time already run, etc.

- Different between `preemption` and `context switch`?
  - `preemption`: Firing of timer interrupt is preempting the current running process and running the interrupt service routine of timer interrupt.
  - `context switch`: What happens when the kernel alters the state of the processor (the registers, mode, and stack) between one process or thread's context and another.
    - [context_switch()](https://github.com/torvalds/linux/blob/master/kernel/sched/core.c) function is called in the kernel.

### Preemption in kernel space and user space

- User space:
  - Under Linux, US programs have always been preemptible: the kernel interrupts US programs to switch to other threads, using the regular clock tick.
  - So the kernel doesn't wait for US programs to explicitly release the processor. This means that an infinite loop in an US program CANNOT block the system.

- Kernel space:
  - Until 2.6 kernels, the kernel itself was not preemptible, as soon as one thread has entered the kernel, it could not be preempted to execute an other thread.
    - The processor could be used to execute another thread when a system call was terminated, or when the current thread explicitly asked the scheduler to run another thread using the `schedule()` function.
    - This means that an infinite loop in the kernel code blocked the entire system.
  - However, this absence of preemption in the kernel caused several problems with regard to latency and scalability.
  - So, kernel preemption has been introduced in 2.6 kernels, and one can enable or disable it using the `CONFIG_PREEMPT` option.
    - An infinite loop in the code can no longer block the entire system.

### When can kernel preemption happen

- When returning to kernel-space from an interrupt handler.
- When kernel code becomes preemptible again.
- If a task in the kernel explicitly calls `schedule()`.
- If a task in the kernel blocks (which results in a call to `schedule()`).

### Example

- First example, while process A executes an exception handler (necessarily in Kernel Mode), a higher priority process B becomes runnable.
  - This could happen, if an IRQ occurs and the corresponding handler awakens process B.
  - As the kernel is preemptive, a forced process switch replace process A with B.
  - The exception handler is left unfinished and will be resumed only when scheduler selects again process A for execution.

- More example, consider a process that executes an exception handler and whose time quantum expires. As the kernel is preemptive, the process may be replaced immediately.

- Motivation for making the kernel preemptive
  - reduce the dispatch latency of the User Mode processes:
    - the delay between the time they become runnable and the time they actually begin running.
  - Processes performing timely scheduled tasks (such as external hw controllers, environmental monitors, movie players, and so on) really benefit from kernel preemption, because it reduces the risk of being delayed by another process running in kernel mode.

## Reentrancy

- What is a kernel `control path`?
  - A kernel control path denotes the sequence of instructions executed by the kernel to handle a system call, an exception, or an interrupt.

- `Reentrant` kernels
  - Linux kernel is reentrant. This means that several processes may be executing in Kernel Mode at the same time.
  - On uniprocessor systems, only one process can progress, but many can be blocked in Kernel Mode when waiting for the CPU or the completion of some I/O operation.
