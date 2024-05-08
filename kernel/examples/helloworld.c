/*
* helloworld.c 
* - Demonstrating the module_init() and module_exit() macros.
* - Illustrating the __init, __initdata and __exit macros.
* - This is preferred over using init_module() and cleanup_module().
* - Demonstrates module documentation.
*/

#include <linux/init.h> /* Needed for the macros */
#include <linux/module.h> /* Needed by all modules */
#include <linux/printk.h> /* Needed for pr_info() */

static int helloworld_data __initdata = 3;

static int __init helloworld_init(void)
{
    pr_info("Hello, world %d.\n", helloworld_data);
    return 0;
}


static void __exit helloworld_exit(void)
{
    pr_info("Goodbye, world.\n");
}

module_init(helloworld_init);
module_exit(helloworld_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("A sample helloworld driver");
