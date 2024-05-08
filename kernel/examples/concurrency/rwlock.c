/* rwlock.c - Read write locks example.
 * If u know for sure that there are no functions triggered by IRQs
 * which could possibly interface with your logic then u can use the
 * simpler `read_lock()` and `read_unlock()` or corresponding functions.
 */

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/rwlock.h>

static DEFINE_RWLOCK(_rwlock);

static void _test_read_lock(void);
static void _test_write_lock(void);

static int __init _rwlock_init(void)
{
    int res = 0;
    pr_info("%s(): invoked.\n", __FUNCTION__);

    _test_read_lock();
    _test_write_lock();

    return res;
}

static void __exit _rwlock_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
}

static void _test_read_lock(void)
{
    unsigned long flags;

    pr_info("%s(): invoked.\n", __FUNCTION__);

    read_lock_irqsave(&_rwlock, flags);

    /* Do something or other safely. Because this uses 100% CPU time,
     * this code should take no more then a few miliseconds to run.
     */

    pr_info("%s(): flags: %lu.\n", __FUNCTION__, flags);

    read_unlock_irqrestore(&_rwlock, flags);
}

static void _test_write_lock(void)
{
    unsigned long flags;

    pr_info("%s(): invoked.\n", __FUNCTION__);

    write_lock_irqsave(&_rwlock, flags);

    /* Do something or other safely. Because this uses 100% CPU time,
     * this code should take no more then a few miliseconds to run.
     */

    pr_info("%s(): flags: %lu.\n", __FUNCTION__, flags);

    write_unlock_irqrestore(&_rwlock, flags);
}

module_init(_rwlock_init);
module_exit(_rwlock_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Read write locks example.");