# Linux Programming Interface

## 2. Fundamental concepts

### 2.1. The Core OS: Kernel

- The term OS is commonly used with two different meanings:
  - 1. To denote the entire package consisting of the central software managing a computer's resources and all of the accompanying standard software tools, such as command-line interpreter, graphical user interfaces, file utilities, and editors.
  - 2. More narrowly, to refer to the central software that manages and allocates computer resources (i.e. the CPU, RAM, and devices).

- The term `kernel` is often used as a synonym for the second meaning, and it is with this meaning of the term `OS`.

- Although it is possible to run programs on a computer without a kernel, the presence of a kernel greatly simplifies the writing and use of other programs, and increases the power and flexibility available to programmers. The kernel does this by providing a software layer to manage the limited resources of a computer.

### 2.1.1. Tasks performed by the kernel

- 1. **Process scheduling**: A computer has one or more central processing units (CPUs), which execute the instructions of programs. Like other UNIX systems, Linux is a **Preemptive Multitask** OS, that means multi-processes can simultaneously reside in memory and each may receive use of the CPU(s). **Preemptive** means that the rules governing which processes receive use of the CPU and how long are determined by the kernel process scheduler.

- 2. **Memory Management**: While computer memories are enormous by the standards of a decade or two ago, the size of software has also correspondingly grown, so that physical memory (RAM) remains a limited resource that the kernel must share among processes in an equitable and efficient fashion.
  - Like most modern OSes, Linux employs virtual memory management, a technique that confers two main advantages:
    - 1. Processes are isolated from one another and from the kernel, so that one process can't read and modify the memory of another process or the kernel.
    - 2. Only part of a process needs to be kept in memory, thereby lowering the memory requirements of each process and allowing more processes to be held in RAM simultaneously. This leads to better CPU utilization, since it increases the likelihood that, at any moment in time, there is at least one process that the CPU(s) can execute.
- 3. **Provision of a file system**: The kernel provides a file system on disk, allowing files to be created, retrieved, updated, deleted, and so on.

- 4. **Creation and termination of processes**: The kernel can load a new program into memory, providing it with the resources (e.g., CPU, memory, and access to files) that it needs in order to run.
  - Such an instance of a running program is termed a `process`.
  - Once a process has completed execution, the kernel ensures that the resources it uses are freed for subsequent reuse by later programs.

- 5. **Access to devices**: The devices (mice, monitors, keyboards, disk and tape drives, and so on) attached to a computer allow communication of information between the computer and the outside world. permitting input, output, or both. The kernel provides programs with an interface that standardizes and simplifies access to devices, while at the same time arbitrating access by multiple processes to each device.

- 6. **Networking**: The kernel transmits and receives network messages (packets) on behalf of user processes. This task including routing of network packets to the target system.

- 7. **Provision of a system call application programming interface (API)**: Processes can request the kernel to perform various tasks using kernel entry points known as **system calls**.

- In addition to the above features, multi-user OSes such as Linux provide users with the abstraction of a *virtual private computer;* that is each user can log on to the system and operate largely independently of other users. For example, each user has their own disk storage space (home directory). In addition, users can run programs, each of which gets a share of the CPU and operates in its own virtual address space, and these programs can independently access devices and transfer information over the network. The kernel resolves potential conflicts in accessing hardware resources, so users and processes are generally unaware of the conflicts.

### 2.1.2. Kernel mode and User mode

- Modern processor architectures typically allow the CPU to operate in at least two different modes: `user mode` and `kernel mode`.
  - **Hardware instructions allow switching from one mode to the other.**
- Correspondingly, areas of virtual memory can be marked as being part of `user space` and `kernel space`. When running in user mode, the CPU can access only memory that is marked as being in user-space; attempts to access memory in kernel space result in a hardware exception. When running in kernel mode, the CPU can access both user and kernel memory space.

- Certain operations can be performed only while the processor is operating in kernel mode. Examples include:
  - Executing the `halt` instruction to stop the system,
  - accessing the memory-management hardware,
  - and initiating device I/O operations.
- By takin advantage of this hardware design to place the operating system in kernel space, operating system implementers can ensure that user processes are not able to access the instructions and data structures of the kernel, or to perform operations that would adversely affect the operation of the system.

### 2.1.3. Process versus kernel views of the system

- A running system typically has numerous processes. For a process, many things happen asynchronously.

- 1. An executing process doesn't know when it will next timeout, which other processes will then be scheduled for the CPU, or when it will next be scheduled.
- 2. The delivery of signals and the occurrence of inter-process communication events are mediated by the kernel, and can occur at any time for a process. Many things happen transparent for a process.
- 3. A process doesn't know where it is located in RAM or, in general whether a particular part of its memory space is currently resident in memory or held in swap area (a reserved area of disk space ued to supplement the computer's RAM).
- 4. Similarly, a process doesn't know where on the disk drive the files it accesses are being held; it simply refers to the files by name.
- 5. A process operates in isolation; it can't directly communicate with another process.
- 6. A process can't itself create a new process or even end its own existence.
- 7. Finally, a process can't communicate directly with the input and output devices attached to the computer.

- By contrast, a running system has one kernel that knows and controls everything. The kernel facilitates the running of all processes on the system.
- 1. The kernel decides which process will next obtain access to the CPU, when it will do so, and for how long.
- 2. The kernel maintains data structures containing information about all running processes and updates these structures as processes are created, change state and terminate.
- 3. The kernel maintains all of the low-level data structures that enable the filenames used by programs to be translated into physical locations on the disk.
- 4. The kernel also maintains data structures that map the virtual memory of each process into the physical memory of the computer and swap areas on disk.
- 5. All communication between processes is done via mechanisms provided by the kernel.
- 6. In response to requests from processes, the kernel creates new processes and terminates existing processes.
- 7. Lastly, the kernel (device drivers) performs all direct communication with input and output devices, transferring information to and from user processes as required.

- When we say things such as `a process can create another process`, `a process can create a pipe`, `a process can write data to a file`, and `a process can terminate by calling exit()`. Remember, however, that the kernel mediates all such actions, and these statements are just shorthand for **A process can `request that the kernel` create another process**, and so on.
