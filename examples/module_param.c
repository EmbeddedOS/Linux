/*
* module_param.c - Demonstrates command line argument passing to a module.
*/

#include <linux/init.h> /* Needed for the macros */
#include <linux/module.h> /* Needed by all modules */
#include <linux/printk.h> /* Needed for pr_info() */
#include <linux/kernel.h> /* for ARRAY_SIZE() */
#include <linux/moduleparam.h>
#include <linux/stat.h>

static long int my_long = 9999;
static char* my_str = "larva";
static int my_int_array[4] = {0, 1, 2, 3};
static int my_int_array_count = 0;
/* module_param(foo, int, 0000)
 * The first param is the parameters name.
 * The second param is its data type.
 * The final argument is the permissions bits,
 * for exposing parameters in sysfs (if non-zero) at a later stage.
 */

module_param(my_long, long, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(my_long, "A long integer");
module_param(my_str, charp, 0000);
MODULE_PARM_DESC(my_str, "A character string");

/* module_param_array(name, type, num, perm);
 * The first param is the parameter's (in this case the array's) name.
 * The second param is the data type of the elements of the array.
 * The third argument is a pointer to the variable that will store the number
 * of elements of the array initialized by the user at module loading time.
 * The fourth argument is the permission bits.
 */

module_param_array(my_int_array, int, &my_int_array_count, 0000);
MODULE_PARM_DESC(my_int_array, "An array of integers");

static int __init helloworld_init(void)
{
    pr_info("my_long: %ld\n", my_long);
    pr_info("my_str: %s\n", my_str);

    for (int i = 0; i < ARRAY_SIZE(my_int_array); i++)
    {
        pr_info("my_int_array[%d]: %d\n", i, my_int_array[i]);
    }

    pr_info("my_int_array_count: %d\n", my_int_array_count);

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
MODULE_DESCRIPTION("A sample module params driver");
