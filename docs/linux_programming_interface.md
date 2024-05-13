# Linux Programming Interface

## 2. Fundamental concepts

### 2.1. The Core OS: Kernel

- The term OS is commonly used with two different meanings:
  - 1. To denote the entire package consisting of the central software managing a computer's resources and all of the accompanying standard software tools, such as command-line interpreter, graphical user interfaces, file utilities, and editors.
  - 2. More narrowly, to refer to the central software that manages and allocates computer resources (i.e. the CPU, RAM, and devices).

- The term `kernel` is often used as a synonym for the second meaning, and it is with this meaning of the term `OS`.

- Although it is possible to run programs on a computer without a kernel, the presence of a kernel greatly simplifies the writing and use of other programs, and increases the power and flexibility available to programmers. The kernel does this by providing a software layer to manage the limited resources of a computer.

#### 2.1.1. Tasks performed by the kernel

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

#### 2.1.2. Kernel mode and User mode

- Modern processor architectures typically allow the CPU to operate in at least two different modes: `user mode` and `kernel mode`.
  - **Hardware instructions allow switching from one mode to the other.**
- Correspondingly, areas of virtual memory can be marked as being part of `user space` and `kernel space`. When running in user mode, the CPU can access only memory that is marked as being in user-space; attempts to access memory in kernel space result in a hardware exception. When running in kernel mode, the CPU can access both user and kernel memory space.

- Certain operations can be performed only while the processor is operating in kernel mode. Examples include:
  - Executing the `halt` instruction to stop the system,
  - accessing the memory-management hardware,
  - and initiating device I/O operations.
- By takin advantage of this hardware design to place the operating system in kernel space, operating system implementers can ensure that user processes are not able to access the instructions and data structures of the kernel, or to perform operations that would adversely affect the operation of the system.

#### 2.1.3. Process versus kernel views of the system

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

### 2.2. The Shell

- A shell is a special-purpose program designed to read commands typed by a user and execute appropriate programs in response to those commands.
- A number of important shells have appeared over time:
  - 1. Bourne shell (sh).
  - 2. C shell (csh).
  - 3. Korn shell (ksh).
  - 4. Bourne again shell (bash): This shell is the GNU project’s re-implementation of the Bourne shell.

### 2.3. Users and Groups

- Each user on the system is uniquely identified, and users may belong to groups.

- Users:
  - Every user has a unique `login name` (username) and a corresponding numeric user ID (UID).
  - For each user, these are defined by a line in the system *password file*, `/etc/passwd`, which includes the additional information:
    - 1. **Group ID**: The numeric group ID of the first of the groups of which the user is a member.
    - 2. **Home Directory**: the initial directory into which the user is placed after logging in.
    - 3. **Login Shell**: the name of the program to be executed to interpret user commands.

- Groups:
  - For administrative purposes - in particular, for controlling access to files and other system resources - it is useful to organize user into `groups`.

- Superuser:
  - One user, known as the `superuser`, has special privileges within the system. The superuser account has the user ID 0, and normally has the login name `root`.
  - On typical Unix system, the superuser bypasses all permission checks in the system.

### 2.4. Single directory Hierarchy, Directories, Links, and Files

- The kernel mains a single hierarchical directory structure to organize all files in the system.
- At the base of this hierarchy is the `root directory`, named `/` (slash). All files and directories and children or further removed descendants of the root directory.

#### 2.4.1. File types

- Within the filesystem, each file is marked with a `type`, indicating what kind of file it is.

- One of these file type denotes ordinary data files, which are usually called `regular` or `plain` files to distinguish them from other file types. These other file types include devices, pipes, sockets, directories, and symbolic links.

- The term `file` is commonly used to denote a file of any type, not just a regular file.

#### 2.4.2. Directories and links

- **A directory is special file** whose contents take the form of a table of filenames coupled with references to the corresponding files. This filename-plus-reference association is called a `link`, and files may have multiple links, and thus multiple names, in the same or in different directories.

- Directories may contain links both to files and to other directories.

- Every directory contains at least two entries: `.` (dot), which is a link to the directory itself, and `..` (dot-dot), which is a link to its `parent directory`.

#### 2.4.3. Symbolic links

- Like a normal link, a `symbolic link` provides an alternative name for a file. But whereas a normal link is a `filename-plus-pointer` entry in a directory list, **a symbolic link is a specially marked file containing the name of another file**. This later file is often called the target of the symbolic link, and it is common to say that the symbolic link `points` or `refers` to the target file.

- When a pathname is specified in a system call, in most circumstances, the kernel automatically *dereferences* each symbolic link in the pathname, replace it with the filename to which it points. This process may happen recursively if the target of a symbolic.

- If a symbolic link refers to a file that doesn't exist, it is said to be a `dangling link`.

- hard-link is normal link.
- soft-link is symbolic link.

#### 2.4.4. File ownership and permissions

- Each file has an associated user IS and group ID that define the owner of the file and the group to which it belongs. The ownership of a file is used to determine the access rights available to users of the file.

### 2.5. File I/O model

- One of the distinguishing features of the I/O model in Unix systems is the concept of `universality of I/O`. This means that the same system calls (`open()`, `read()`, `write()`, `close()` and so on) are used to perform I/O on all types of files, including devices. ( The kernel translates the application's I/O requests into appropriate file-system or device-driver operations that perform I/O on the target file or device).
- Thus, a program employing these system calls will work on any type of file.

#### 2.5.1. File descriptors

- The I/O system calls refer to open files using a `file descriptor`, a (usually small) non-negative integer.
- A file descriptor is typically obtained by a call to `open()`, which takes a pathname argument specifying a file upon which I/O is to be performed.
- Normally, a process inherits three open file descriptors when it is started by the shell:
  - 1. descriptor 0 is standard input.
  - 2. descriptor 1 is standard output.
  - 3. descriptor 2 is standard error.

#### 2.5.2. The `stdio` library

- To perform file I/O, C programs typically employ I/O functions contained in the standard C library. This set of functions, referred to as the `stdio` library, includes `fopen()`, `fclose()`, `scanf()`, `printf()`, `fgets()`, `fputs()` and so on. The `stdio` functions are layered on top of the I/O system calls (`open()`, `close()`, `read()`, `write()` and so on).

### 2.6. Programs

- Programs normally exist in two forms. The first form is `source code`, human-readable. To be executed, source code must be converted to the second form: `binary` machine language instructions that the computer can understand.
- The two meanings of the term `program` are normally considered synonymous.

#### 2.6.1. Filters

- A `filter` is the name often applied to a program that reads its input from `stdin`, performs some transformation of that input, and writes the transformed data to `stdout`. Examples of filters include `cat`, `grep`, `tr`, `sort`, `wc`, `sed`, and `awk`.

#### 2.6.2. Command-line arguments

- In C, programs can access the command-line arguments, the words that are supplied in the command line when the program is run. To access the command arguments, the main function of the program is declared as follows:

```C
int main(int argc, char *argv[])
```

### 2.7. Processes

- Put most simply, a `process` is an instance of an executing program. When a program is executed, the kernel loads the code of the program into virtual memory, allocates space for program variables, and sets up kernel bookkeeping data structures to record various information (such as process ID, termination status, user IDs, and group IDs) about the process.

- From a kernel point of view, processes are the entities among which the kernel must share the various resource of the kernel.
- For resources that are limited, such as memory, the kernel initially allocates some amount of the resource to the process, and adjusts this allocation over the lifetime of the process in responsible to the demands of the process and overall system demand for that resource.
- When process terminates, all such resources are released for reuse by other processes.
- Other resources, such as the CPU and network bandwidth, are renew-able, but must be shared equitably among all processes.

#### 2.7.1. Process memory layout

- A process is logically divided into the following parts, known as `segments`:
  - 1. `Text`: the instructions of the program.
  - 2. `Data`: the static variables used by the program.
  - 3. `Heap`: an area from which programs can dynamically allocate extra memory.
  - 4. `Stack`: a piece of memory that grows and shrinks as functions are called and return and that is used to allocate storage for local variables and function call linkage information.

#### 2.7.2. Process creation and program execution

- A process can create a new process using `fork()` system call. The process that calls `fork()` is referred to as the `parent process`, and the new process is referred to as `child process`.

- The kernel creates the child process by making a duplicate of the parent process. The child inherits copies of the parent's data, stack, and heap segments, which it may then modify independently of the parent's copies. (The program text, which is placed in memory marked as read-only, is shared by the two processes).

- The child process goes on either to execute a different set of functions in the same code as the parent, or frequently, to use the `execve()` system call to load and execute an entirely new program. An `execve()` call destroys the existing text, data, stack and heap segments, replacing them with new segments based on the code of the new program.

#### 2.7.3. Process ID and parent process ID

- Each process unique integer `process identifier (PID)`. Each process also has a `parent process identifier (PPID)` attribute, which identifies the process that requested the kernel to create this process.

#### 2.7.4. Process termination and termination status

- A process can terminate in one of two ways:
  - 1. By requesting its own termination using the `_exit()` system call (or the related `exit()` lib function),
  - 2. Or by being killed by the delivery of a signal.

- In either case, the process yields a `termination status`, a small non-negative integer value that is available for inspection by the parent process using the `wait()` system call.

- In the case of a call to `_exit()`, the process explicitly specifies its own termination status.
- If a process is killed by a signal, the termination status is set according to the type of signal that caused the death of the process.

- By convention, a termination status of 0 indicates that process succeeded, and a nonzero status indicates that some error occurred.

- Most shells make the termination status of the last executed program available via a shell variable named `$?`.

#### 2.7.5. Process user and group identifiers (credentials)

- Each process has a number of associated user IDs (UIDs) and group IDs (GIDs). These include:
  - `Real user ID` and `real group ID`: These identify the user and group to which the process belongs. A new process inherits these IDs from its parent. A login shell gets its real user ID and real group ID from the corresponding fields in the system password file.
  - `Effective user ID` and `Effective group ID`: These two IDs (in conjunction with the supplementary groups IDs discussed in a moment) are used in determining the permission that process has when accessing protected resources such as files and inter-process communication objects.
    - Typically, `Effective user IS` has the same values as the corresponding real IDs.

#### 2.7.6. Privileged Processes

- Traditionally, on Unix systems, a `privileged process` is one whose `effective user ID` is 0 (superuser).
- A process may be privileged because it was created by another privileged process - for example, by a login shell started by root (superuser).
- Another way a process may become privileged is via the set-user-ID mechanism, which allows a process to assume an effective user ID that is the same as the user ID of the program file that it is executing.

#### 2.7.7. Capabilities

- Linux divides the privileges traditionally accorded to the super user into a set of distinct units called `capabilities`. Each privileged operation is associated with a particular capability, and a process can perform an operation only if it has the corresponding capability.

#### 2.7.8. The `init` process

- When booting the system, the kernel creates a special process called `init`, the `parent of all processes`, which is derived from the program file (`sbin/init`).

- All processes on the system are created (using `fork()`) either by `init` or by one of its descendants.

- The init process always has the process ID `1` and runs with superuser privileges.

- The `init` process **CAN NOT** be killed (not even by the superuser), and it terminates only when the system is shutdown.

- **The main task of `init` is to create and monitor a range of processes required by a running system**.

#### 2.7.9. Daemon Processes

- A `daemon` is a special-purpose process that is created and handled by the system in the way as other processes, but which is distinguished by the following characteristics:
  - 1. It is long-lived. A daemon process is often started at system boot and remains in existence until the system is shutdown.
  - 2. It runs in the background, and has no controlling terminal from which it can read input or to which it can write output.

- Examples of daemon processes include `syslogd`, which records messages in the system log, and `httpd`, which serves web pages.

#### 2.7.10. Environment list

- Each process has an `environment list`, which is a set of `environment variables` that are maintained within the user-space memory of the process.
- Each element of this list consists of a name an associated value.

- When a new process is created via `fork()`, it inherits a copy of it's parent's environment. Thus, the environment provides a mechanism for a parent process to communicate information to a child process.

- When a process replaces the program that it is running `exec()`, the new program either inherits the environment used by the old program or receives a new environment specified as part of the `exec()` call.

- Environment variables are created with the `export` command in most shells, as in the following example:

```bash
export VAR="Hello world"
```

- C program can access the environment using an external variable `char **environ`.

- Environment variables are used for variety of purposes. For example, the shell defines and uses a range of variables that can be accessed by scripts and programs executed from the shell.

#### 2.7.11. Resource limits

- Each process consumes resources, such as open files, memory and CPU time. Using `setlimit()` system call, a process can establish upper limits on its consumption of various resources.

- When a new process is created with `fork()`, it inherits copies of its parent's resource limit settings.

- The resource limits of the shell can be adjusted using the `ulimit` command. These limit settings are inherited by the child processes that shell creates to execute commands.
