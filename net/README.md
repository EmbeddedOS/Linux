# Net link sockets - IPC between user space and kernel space

## Computer architecture

    Application 1 <----IPC----> Application 2
        ||
    Net link sockets,
       IOCTL,
    device files,
     system calls
        ||
        \/
      Kernel
        ||
    Device driver
        ||
        \/
    Hardware (CPU, Memory, Devices, etc.)

- Note: All device drivers could be LKM, but all LKMs are not device driver.

## Socket as a unified interface

- Net-link sockets are especially created to facilitate clean bidirectional communication between user space and kernel space.

- Other techniques can also be used for US <---> KS communication, but they were not invented for this purpose.
  - E.g: ioctl, device files, system calls.
    - If you want to make new system call to communication between US and KS, you have to rebuild the kernel.
    - U just don't write system call unless u have a very fairly good reason because system calls are actually general purpose calls, and those should be used by every other application that is running on the Linux platform just to meet the requirement of one application.

    - Device files have been invented, especially to write device drivers.

- A socket based technique was developed to build the unified interface using which user space application (USA) can interact with various kernel subsystems.

- We usually create a socket using `socket` system call:

```C
/*
 * These 3 arguments determine:
 * 1. Socket address family.
 * 2. Communication type: data-gram based or stream based.
 * 3. Protocol used for communication.
 */

int fd = socket(AF, socket_type, protocol_id);
```

- Thus, socket interface is unified - depending on arguments passed, we set up communication properties - whom to communicate, what to communicate, how to communicate.

## Net-link communication use cases

App 1 <-(Unix domain)-> App 2  <-(Unix domain)-> App 3 <-(Network domain)-> Remote App 1

----------------------------------------------------
    Socket interface (bunch of system calls)
    (socket, accept, bind, send, recv, close, etc.)
----------------------------------------------------
                        ||
                    (net-link domain)
                        ||
                        \/
Kernel subsystem 1--Kernel subsystem 2--Kernel subsystem 3

- To perform any socket based communication, the Linux kernel provides us the bunch of system calls collectively, which are called as `socket interface`.
  - There are APIs like: accept, bind, send, recv, close, etc.
  - If u have ever written any socket program, u must have used these APIs in order to write your socket based communication.

- It might be possible that the app wants to communicate with some remote machine present elsewhere on the network. So for that the app will establish two network socket based communication using the same socket interface: **network domain sockets**.

- In another example, the app may wants to interact with another application running on the same system. For that, the socket interface that is provided for setting up the communication between two apps: **Unix domain sockets**.

- And finally, the application may be entrusted with the communication with any kernel subsystem lying on the kernel space for that: **link sockets** is designed for that.

- Again the application make use of the same socket interface that is same bunch of system calls in order to set up, net-link based communication with any kernel subsystems running in the kernel space.

- For example kernel subsystems: `routing TCP/IP stack`, `firewall- IP tables`, `ARP tables, interface properties`, etc.

## Our net-link

- We shall explore `net-link socket based communication` between Us - KS by developing user app which interact with our LKM which is in-char of our `Routing Subsystem of Kernel`.

----------- User Space app--------------------
            /\          ||
            ||          \/
----Kernel Routing table manager Subsystem----

- User actions:
  - perform CRUD operations on this routing table.

- Kernel actions:
  - LKM will be in charge of routing the table data structure.
  - it shall receive and process CRUD orders coming from application.

- Same net-link communication semantics applies to other kernel subsystems.

## Net-link message format

- US and KS exchange net-link messages in well defined format.
- Any net-link message going from US to KS or from KS to US must be as per net-link standard message format.

- A typical net-link message is laid out in memory as below:

|---header---|padding|-----payload---|
<---16bytes-->

```C
struct nlmsghdr {
    __u32 nlmsg_len;      /* Length of message including header */
    __u16 nlmsg_type;     /* Message content */
    __u16 nlmsg_flags;    /* Additional flags */
    __u32 nlmsg_seq;      /* Sequence number */
    __u32 nlmsg_pid;      /* Sending process port ID */
};
```

- Both parties, can exchange **Multiple Net-link** message units cascaded one after the other.

### Net-link message types

- 4 standard types are defined in `/usr/include/linux/netlink.h`

```C
#define NLMSG_NOOP              0x1     /* Nothing.             */
#define NLMSG_ERROR             0x2     /* Error                */
#define NLMSG_DONE              0x3     /* End of a dump        */
#define NLMSG_OVERRUN           0x4     /* Data lost            */

#define NLMSG_MIN_TYPE          0x10    /* < 0x10: reserved control messages */
```

- `NLMSG_NOOP`: When the other party receives this message, it does nothing except it replies with `NLMSG_DONE` telling the sender that all is fine (= Is all ok? same is PING).
- `NLMSG_ERROR`: When the party receives this message as a reply to the message sent previously, it means that other party failed to perform requested action (= negative feedback).
- `NLMSG_DONE`: This the **last net-link message** in the cascade of multiple net-link message units.
  - This message is known as an acknowledgement of the previous request.
- `NLMSG_OVERRUN`: Currently not used in linux kernel anywhere.

- Note: besides above, user can define his own message types which should be >= 16.

### Net-link message flags

- These flags are set in Net-link message to convey additional information to the recipient.
- Multiple flags could be set using bitwise AND/OR operators.

```C
/* Flags values */

#define NLM_F_REQUEST           0x01    /* It is request message.       */
#define NLM_F_MULTI             0x02    /* Multipart message, terminated by NLMSG_DONE */       
#define NLM_F_ACK               0x04    /* Reply with ack, with zero or error code */
#define NLM_F_ECHO              0x08    /* Echo this request            */
#define NLM_F_DUMP_INTR         0x10    /* Dump was inconsistent due to sequence change */      
#define NLM_F_DUMP_FILTERED     0x20    /* Dump was filtered as requested */

/* Modifiers to GET request */
#define NLM_F_ROOT      0x100   /* specify tree root    */
#define NLM_F_MATCH     0x200   /* return all matching  */
#define NLM_F_ATOMIC    0x400   /* atomic GET           */
#define NLM_F_DUMP      (NLM_F_ROOT|NLM_F_MATCH)

/* Modifiers to NEW request */
#define NLM_F_REPLACE   0x100   /* Override existing            */
#define NLM_F_EXCL      0x200   /* Do not touch, if it exists   */
#define NLM_F_CREATE    0x400   /* Create, if it does not exist */
#define NLM_F_APPEND    0x800   /* Add to end of list           */

/* Modifiers to DELETE request */
#define NLM_F_NONREC    0x100   /* Do not delete recursively    */

/* Flags for ACK message */
#define NLM_F_CAPPED    0x100   /* request was capped */
#define NLM_F_ACK_TLVS  0x200   /* extended ACK TVLs were included */
```

- Flags:
  - `NLM_F_REQUEST`: This message contains a request. Should be set for each message from US -> KS, If not kernel replies back with invalid argument `EINVAL` error. This US as Master, and Kernel as Slave.
  - `NLM_F_CREATE`: US asking kernel subsystem to create a resource or configuration.
  - `NLM_F_EXCL`: Used together `NLM_F_CREATE`, US asking kernel to return an error with if the configuration/resource already exists.
  - `NLM_F_REPLACE`: US wants to replace an existing configuration in the KS subsystem.
  - `NLM_F_APPEND`: US application requesting KS add more data to existing configuration, for example adding some data to existing linked list.
  - `NLM_F_DUMP`: US requesting Ks to send itself all the data of particular type. KS replies with multipart cascaded net-link messages to such request from US.
  - `NLM_F_MULTI`: This flag is set to tell the recipient that there is NEXT net-link message following to this one.
  - `NLM_F_ACK`: If set, US is requesting the KS to reply back with the confirmation message of the US request. KS replies with `NLMSG_NOOP` or `NLMSG_ERROR` type.
