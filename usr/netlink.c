#include <linux/netlink.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <memory.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>

#define NLMSG_LAVA 20       /* User defined NL MSG TYPES, should be > 16. */
#define NETLINK_LAVA_PROTOCOL 31 /* A Unused Unreserved net-link protocol number.*/
#define LINUX_KERNEL_DEFAULT_PORT_ID 0
#define MSG_MAXSIZE 512

int start_handle_callback_msg_thread(int fd);
void* receiver_kernel_msg(void*);
int send_netlink_msg_to_kernel(int fd, char* msg, size_t size, int nlmsg_type, uint16_t flags);
int send_Lava_msg(int fd, char* msg, size_t size);

int main()
{
    /* Creates an endpoint for communication and returns a file
     * descriptor that refers to that endpoint.
     * @domain: The domain argument specifies a communication domain.
     * @type: The socket has the indicated type, which specifies the
     * communication semantics.
     * @protocol: specifies a particular protocol to be used with the socket.
     * 
     * PF_NETLINK: The packet family.
     * AF_NETLINK: The address family.
     * SOCK_RAW: Provides raw network protocol access.
     */
    int socket_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_LAVA_PROTOCOL);
    if (socket_fd < 0)
    {
        printf("Failed to create a socket endpoint.");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_nl u_addr; /* User address. */
    memset(&u_addr, 0 ,sizeof(u_addr));
    u_addr.nl_family = AF_NETLINK;
    u_addr.nl_pid = getpid();


    if (bind(socket_fd, (struct sockaddr*)&u_addr, sizeof(u_addr)) < 0)
    {
        printf("Failed to bind the socket address, error: %d", errno);
        exit(EXIT_FAILURE);
    }

    start_handle_callback_msg_thread(socket_fd);

    while(1)
    {
        char msg[20] = "I am Lava.";
        send_Lava_msg(socket_fd, msg, sizeof(msg));

        sleep(5);
    }

    exit(EXIT_SUCCESS);
}

void* receiver_kernel_msg(void* p)
{
    int fd = (int)p;

    /* Make a buffer to store message from kernel. */
    int header_maxsize = NLMSG_HDRLEN + NLMSG_SPACE(MSG_MAXSIZE);
    struct msghdr received_msg_headers;
    struct iovec iov;
    struct nlmsghdr *nlh_recv = (struct nlmsghdr *)calloc(1, header_maxsize);


    while(1)
    {
        int res = 0;

        /* Reset the buffer for new message. */
        memset(nlh_recv, 0, header_maxsize);
        iov.iov_base = (void *)nlh_recv;
        iov.iov_len = header_maxsize;

        memset(&received_msg_headers, 0, sizeof(received_msg_headers));

        received_msg_headers.msg_name = NULL;    /* Socket name. */
        received_msg_headers.msg_namelen = 0;  /* Length of name. */
        received_msg_headers.msg_iov = &iov; /* Data blocks. */
        received_msg_headers.msg_iovlen = 1; /* Number of blocks. */

        /* Receive a message from a socket.
         * @socket: specifies the socket file descriptor.
         * @message: points to a msghdr structure,
         * contain both buffer to store the source address 
         * and the buffers for the incoming message.
         * @flags: Specifies the type of message reception.
         * 
         * The recvmsg() function shall receive messages from
         * unconnected or connected sockets and shall return
         * the length of the message.
         * 
         * If no messages are available at the socket and
         * O_NONBLOCK is not set on the socket's file descriptor,
         * recvmsg() shall block until a message arrives.
         */
        res = recvmsg(fd, &received_msg_headers, 0);

        if (res <= 0)
        {
            printf("Failed to receive message from kernel.\n");
        } else {
            char* ptr = (char*) NLMSG_DATA(received_msg_headers.msg_iov->iov_base);
            printf("Kernel reply: %s.\n", ptr);
        }

        sleep(3);
    }

    return 0;
}

int start_handle_callback_msg_thread(int fd)
{
    /* Make a detach thread. */
    int res = 0;
    pthread_attr_t attr;
    pthread_t receive_packet_thread;

    /* initialize thread attributes object
     * that is used to create a single thread.
     * @attr: thread attributes object.
     *
     * If the program is run with no command-line argument,
     * then it passes NULL as the attr argument of pthread_create(),
     * so that the thread is created with default attributes.
     *
     * On success, these functions return 0; 
     * on error, they return a nonzero error number.
     */
    res = pthread_attr_init(&attr);
    if (res != 0)
    {
        printf("Failed to init thread attributes.\n");
        goto out;
    }

    /* function sets the detach state attribute of the thread
     * attributes object referred to by attr to the value specified
     * in detachstate.
     * @attr: thread attributes object.
     * @detachstate: detach state attribute (PTHREAD_CREATE_DETACHED, PTHREAD_CREATE_JOINABLE).
     * 
     * On success, these functions return 0;
     * on error, they return a nonzero error number.
     */
    res = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (res != 0)
    {
        printf("Failed to set detach attribute.\n");
        goto out;
    }

    /* Create a new thread.
     * @thread: thread object.
     * @attr: thread attribute object.
     * @start_routine: the function that run on another thread.
     * @arg: arg sent to start_routine function.
     * 
     * On success, pthread_create() returns 0; on error, it returns an
     * error number, and the contents of *thread are undefined.
     */
    res = pthread_create(&receive_packet_thread, &attr, receiver_kernel_msg, (void*)fd);

out:
    return res;
}

int send_Lava_msg(int fd, char* msg, size_t size)
{
    /* Currently, the kernel just captures the NLM_F_ACK messages. */
    return send_netlink_msg_to_kernel(fd, msg, size, NLMSG_LAVA, NLM_F_ACK);
}

int send_netlink_msg_to_kernel(int fd, char* msg, size_t payload_size, int nlmsg_type, uint16_t flags)
{

    /* 1. Specify socket address. */
    struct sockaddr_nl k_addr; /* Kernel address. */
    memset(&k_addr, 0 ,sizeof(k_addr));
    k_addr.nl_family = AF_NETLINK;
    k_addr.nl_pid = LINUX_KERNEL_DEFAULT_PORT_ID;

     /* 2. Make message header + payload. */
    size_t message_size = NLMSG_HDRLEN + NLMSG_SPACE(payload_size);
    
    /* Always use the macros NLMSG_HDRLEN, NLMSG_SPACE(len) to calculate the size of header
     * and payload data. These macros will take care to do necessary alignment.
     */
    struct nlmsghdr *header = (struct nlmsghdr *)calloc(1, message_size);

    header->nlmsg_len = message_size;
    header->nlmsg_pid = getpid();
    header->nlmsg_flags = flags;
    header->nlmsg_type = nlmsg_type;
    header->nlmsg_seq = 0;

    /* NLMSG_DATA(nlh) get pointer to the payload of message.
     */
    strncpy(NLMSG_DATA(header), msg, payload_size);


    /* A vector element. Normally, this structure is used as an array
     * of multiple elements.
     *
     * @iov_base: the pointer member iov_base points to a buffer that
     * is receiving data for readv or is transmitting data for writev.
     * 
     * @iov_len: The member iov_len in each case determines the maximum
     * receive length and the actual write length, respectively.
     * 
     */
    struct iovec iov;
    iov.iov_base = (void *)header;
    iov.iov_len = header->nlmsg_len;

    static struct msghdr message_header;

    memset(&message_header, 0, sizeof(struct msghdr));

    message_header.msg_name = (void *)&k_addr;    /* Socket name. */
    message_header.msg_namelen = sizeof(k_addr);  /* Length of name. */
    message_header.msg_iov = &iov; /* Data blocks. */
    message_header.msg_iovlen = 1; /* Number of blocks. */

    /* 3. Send the message to the socket. */

    /* Send a message on a socket.
     * @sockfd: file descriptor of the sending socket.
     * @msg: the message header structure.
     * @flags: Specifying one or more of the following flags.
     * 
     * With a zero flags argument, sendmsg() is equivalent to write().
     * 
     * On success, these calls return the number of characters sent.
     * On error, -1 is returned, and errno is set appropriately.
     */
    int rc = sendmsg(fd, &message_header, 0);

    if (rc < 0)
    {
        printf("Failed to send message, error: %d", errno);
    }

    free(header);

    return rc;
}
