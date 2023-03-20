

#include <linux/init.h>         /* Needed for the macros. */
#include <linux/module.h>       /* Needed by all modules. */
#include <linux/printk.h>       /* Needed for pr_info(). */
#include <linux/netlink.h>      /* Needed for Netlink kernel configuration structure. */
#include <net/sock.h>           /* Needed for sock structure. */
#include <linux/string.h>

#include "calls.h"

#define OK 0
#define NETLINK_LAVA_PROTOCOL 31 /* A Unused Unreserved net-link protocol number.*/

struct sock* _socket = NULL;

struct netlink_kernel_cfg _cfg = {
    .input = netlink_recv_msg_f
};

static int __init netlink_init(void)
{
    int res = OK;

    /* Make a new custom net-link family protocol.
     * First argument: `init_net` is a global variable which is provided 
     * by the kernel, it represent the complete networking subsystem.
     *
     */
    _socket = netlink_kernel_create(&init_net, NETLINK_LAVA_PROTOCOL, &_cfg);
    if (_socket == NULL)
    {
        pr_err("Net-link socket for protocol: %u failed.", NETLINK_LAVA_PROTOCOL);
        res = -ENOMEM;
        goto out;
    }

    pr_info("Net link driver is loaded.\n");

out:
    return res;
}

static void __exit netlink_exit(void)
{
    /* Release the socket.
     *
     */
    netlink_kernel_release(_socket);
    _socket = NULL;

    pr_info("Net link driver is unloaded.\n");
}

module_init(netlink_init);
module_exit(netlink_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Net link driver.");
