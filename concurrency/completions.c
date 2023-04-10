/* completions.c - Start two thread, but one needs to start before another.
 */

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/err.h>  /* For IS_ERR(). */


static int __init _completion_init(void)
{
    int res = 0;
    pr_info("%s(): invoked.\n", __FUNCTION__);


    return res;
}

static void __exit _completion_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
}

module_init(_completion_init);
module_exit(_completion_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Completions example.");