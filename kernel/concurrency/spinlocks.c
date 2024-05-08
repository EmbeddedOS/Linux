/* spinlocks.c - Spinlocks dynamic-static examples.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/printk.h>

static DEFINE_SPINLOCK(_static_spinlock);
static void _test_static_spinlock(void);

static spinlock_t _dynamic_spinlock;
static void _test_dynamic_spinlock(void);


static int __init _spinlocks_init(void)
{
    int res = 0;
    pr_info("%s(): invoked.\n", __FUNCTION__);

    _test_dynamic_spinlock();

    _test_static_spinlock();

    return res;
}

static void __exit _spinlocks_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
}

static void _test_static_spinlock(void)
{
    unsigned long flags;

    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* spin_lock_irqsave - disables interrupts (on the local processor only)
     * before taking the spinlock; the `previous interrupt state` is stored in flags.
     */
    spin_lock_irqsave(&_static_spinlock, flags);

    /* Do something or other safely. Because this uses 100% CPU time,
     * this code should take no more then a few miliseconds to run.
     */

    pr_info("%s(): flags: %lu.\n", __FUNCTION__, flags);

    spin_unlock_irqrestore(&_static_spinlock, flags);
}

static void _test_dynamic_spinlock(void)
{
    unsigned long flags;

    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* spin_lock_init - produces initialization of the given spinlock.
     */
    spin_lock_init(&_dynamic_spinlock);

    spin_lock_irqsave(&_dynamic_spinlock, flags);

    /* Do something or other safely. Because this uses 100% CPU time,
     * this code should take no more then a few miliseconds to run.
     */

    pr_info("%s(): flags: %lu.\n", __FUNCTION__, flags);

    spin_unlock_irqrestore(&_dynamic_spinlock, flags);
}

module_init(_spinlocks_init);
module_exit(_spinlocks_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Spinlocks dynamic-static examples.");