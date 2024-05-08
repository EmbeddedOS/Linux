/* atomic.c - Atomic operations example.
 */

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/atomic.h>
#include <linux/bitops.h>

static void _test_atomic_adding(void);
static void _test_atomic_subtracting(void);
static void _test_atomic_bitwise(void);

static int __init _atomic_init(void)
{
    int res = 0;
    pr_info("%s(): invoked.\n", __FUNCTION__);

    _test_atomic_adding();
    _test_atomic_subtracting();
    _test_atomic_bitwise();

    return res;
}

static void __exit _atomic_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
}

static void _test_atomic_adding(void)
{
    atomic_t var = ATOMIC_INIT(50);

    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* Add 7.*/
    atomic_add(7, &var);

    /* Add one.*/
    atomic_inc(&var);

    pr_info("%s(): atomic variable is %d.\n", __FUNCTION__ , atomic_read(&var));
}

static void _test_atomic_subtracting(void)
{
    atomic_t var;

    pr_info("%s(): invoked.\n", __FUNCTION__);

    atomic_set(&var, 50);

    /* Add 7.*/
    atomic_sub(7, &var);

    /* Add one.*/
    atomic_dec(&var);

    pr_info("%s(): atomic variable is %d.\n", __FUNCTION__ , atomic_read(&var));
}

static void _test_atomic_bitwise(void)
{
    unsigned long word = 0;
    set_bit(3, &word);
    clear_bit(5, &word);
    change_bit(3, &word);
}

module_init(_atomic_init);
module_exit(_atomic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Atomic operations example.");