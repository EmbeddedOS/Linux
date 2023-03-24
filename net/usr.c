#include <linux/netlink.h>

#include <pthread.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <memory.h>

#define LINUX_KERNEL_DEFAULT_PORT_ID 0
#define MSG_MAXSIZE 512

int send_netlink_msg_to_kernel(int fd, char* msg, size_t size, int nlmsg_type, uint16_t flags);

int main()
{

}

int send_netlink_msg_to_kernel(int fd, char* msg, size_t payload_size, int nlmsg_type, uint16_t flags)
{
    struct sockaddr_nl k_addr;
    memset(&k_addr, 0 ,sizeof(k_addr));
    k_addr.nl_family = AF_NETLINK;
    k_addr.pid = LINUX_KERNEL_DEFAULT_PORT_ID;

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
}
