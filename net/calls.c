#include <linux/printk.h>       /* Needed for pr_info(). */
#include <linux/netlink.h>      /* Needed for Netlink kernel configuration structure. */
#include <linux/skbuff.h>       /* Needed for socket buffer structure. */
#include <linux/kernel.h>       /* For scnprintf(). */
#include <net/sock.h>           /* Network namespace and socket Based APIs. */

#include <linux/string.h>

#include "calls.h"

#define NLMSG_LAVA 20       /* User defined NL MSG TYPES, should be > 16. */
#define REPLY_MAXSIZE 512
#define LINUX_KERNEL_DEFAULT_PORT_ID 0

static void __nlmsg_dump(struct nlmsghdr *nlh);
static inline char * __netlink_get_msg_type(__u16 nlmsg_type);

extern struct sock* _socket;

/* Callback func receive the data coming from
 * the user space.
 * This function receive a socket buffer structure.
 */
void netlink_recv_msg_f(struct sk_buff *skb)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* 1. Get message headers. */
    struct nlmsghdr *netlink_headers;
    netlink_headers = (struct nlmsghdr *)skb->data;

    __nlmsg_dump(netlink_headers);

    /* 2. Get message payload. */
    char* data;
    int data_length;

    data_length = skb->len;
    data = (char*)nlmsg_data(netlink_headers); /* Library function that return head of message payload. */

    pr_info("Msg: %s from %d\n.", data, netlink_headers->nlmsg_pid);

    /* 3. Send reply to the user. */
    char reply[REPLY_MAXSIZE];
    memset(reply, 0, sizeof(reply));

    if (netlink_headers->nlmsg_flags & NLM_F_ACK)
    {
        snprintf(reply, sizeof(reply), "The msg from %d is handled", netlink_headers->nlmsg_pid);
    }

    /*3.1. Make a output socket buffer that will send back to the user. */

    /* nlmsg_new - Allocate a new netlink message.
     * @payload: size of the message payload
     * @flags: the type of memory to allocate.
     */
    struct sk_buff* socket_buffer_out = nlmsg_new(sizeof(reply), 0);

    /*3.2. Push message headers to the output socket buffer. */

    /* nlmsg_put - Add a new netlink message to an skb.
     * @skb: socket buffer to store message in.
     * @portid: netlink PORTID of requesting application.
     * @seq: sequence number of message.
     * @type: message type.
     * @payload: length of message payload.
     * @flags: message flags.
     * 
     * Returns NULL if the tailroom of the skb is insufficient to store
     * the message header and payload.
     */
    struct nlmsghdr * netlink_reply_headers = nlmsg_put(socket_buffer_out,
                                                       LINUX_KERNEL_DEFAULT_PORT_ID,
                                                        netlink_headers->nlmsg_seq,
                                                        NLMSG_DONE,
                                                        sizeof(reply),
                                                        0);

    /*3.2. Push payload pointer to the output socket buffer. */
    strncpy(nlmsg_data(netlink_reply_headers), reply, sizeof(reply));
    
    /*3.3. Send message back to the user. */

    /* nlmsg_unicast - unicast a netlink message.
     * @sk: netlink socket to spread message to.
     * @skb: netlink message as socket buffer.
     * @portid: netlink portid of the destination socket.
     */
    int res = nlmsg_unicast(_socket, socket_buffer_out, netlink_headers->nlmsg_pid);
    if (res < 0)
    { /* If sending successfully, the output socket buffer internally
       * will be consumed or freed by the kernel.
       */

        pr_err("Failed to send message back to the user.\n");
        kfree_skb(socket_buffer_out);
    }


}

/* Helper function that print all message headers.
 */
static void __nlmsg_dump(struct nlmsghdr *nlh)
{
    if (nlh == NULL)
    {
        return;
    }

    pr_info("nlmsg_len: %d, nlmsg_type: %s, nlmsg_flags: %d, nlmsg_seq: %d, nlmsg_pid%d",
            nlh->nlmsg_len,
            __netlink_get_msg_type(nlh->nlmsg_type),
            nlh->nlmsg_flags,
            nlh->nlmsg_seq,
            nlh->nlmsg_pid);
}

/* Helper function that convert net-link header - type from u16 to string.
 */

static inline char * __netlink_get_msg_type(__u16 nlmsg_type)
{
    switch(nlmsg_type)
    {
        case NLMSG_NOOP:
            return "NLMSG_NOOP";
        case NLMSG_ERROR:
            return "NLMSG_ERROR";
        case NLMSG_DONE:
            return "NLMSG_DONE";
        case NLMSG_OVERRUN:
            return "NLMSG_OVERRUN";
        case NLMSG_LAVA:
            return "NLMSG_LAVA";
        default:
            return "NLMSG_UNKNOWN";
    }
}
