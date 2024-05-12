# Linux Programming Interface

## 2. Fundamental concepts

### 2.1. The Core OS: Kernel

- The term OS is commonly used with two different meanings:
  - 1. To denote the entire package consisting of the central software managing a computer's resources and all of the accompanying standard software tools, such as command-line interpreter, graphical user interfaces, file utilities, and editors.
  - 2. More narrowly, to refer to the central software that manages and allocates computer resources (i.e. the CPU, RAM, and devices).

- The term `kernel` is often used as a synonym for the second meaning, and it is with this meaning of the term `OS`.

- Although it is possible to run programs on a computer without a kernel, the presence of a kernel greatly simplifies the writing and use of other programs, and increases the power and flexibility available to programmers. The kernel does this by providing a software layer to manage the limited resources of a computer.

- Tasks performed by the kernel:
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
