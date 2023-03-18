/*
* helloworld.c - Demonstrating the module_init() and module_exit() macros.
* This is preferred over using init_module() and cleanup_module().
*/

#include <linux/init.h> /* Needed for the macros */
#include <linux/module.h> /* Needed by all modules */
#include <linux/printk.h> /* Needed for pr_info() */

static int __init helloworld_init(void)
{
    pr_info("Hello, world.\n");
    return 0;
}


static void __exit helloworld_exit(void)
{
    pr_info("Goodbye, world.\n");
}

module_init(helloworld_init);
module_exit(helloworld_exit);

MODULE_LICENSE("GPL");