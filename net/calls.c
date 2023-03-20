#include <linux/printk.h>       /* Needed for pr_info(). */
#include <linux/netlink.h>      /* Needed for Netlink kernel configuration structure. */
#include "calls.h"


static void __nlmsg_dump(struct nlmsghdr *nlh);

/* Callback func receive the data coming from
 * the user space.
 */
void netlink_recv_msg_f(struct sk_buff *skb)
{
}

/* Helper function that print all message headers.
 */
static void __nlmsg_dump(struct nlmsghdr *nlh)
{
    if (nlh == NULL)
    {
        return;
    }

    pr_info("nlmsg_len: %u, nlmsg_type: %u, nlmsg_flags: %u, nlmsg_seq: %u, nlmsg_pid%u",
            nlh->nlmsg_len,
            nlh->nlmsg_type,
            nlh->nlmsg_flags,
            nlh->nlmsg_seq,
            nlh->nlmsg_pid);
}