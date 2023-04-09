/* mutex.c - mutual exclusion example.
 */

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/printk.h>

static DEFINE_MUTEX(_mutex);

static int __init _mutex_init(void)
{
    int res = 0;
    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* Return 1 if the mutex has been acquired successfully, and 0 on contention.
     */
    res = mutex_trylock(&_mutex);
    if (res != 0)
    {
        /* mutex_is_locked - is the mutex locked.
         * @lock: the mutex to queried.
         * 
         * Returns true if the mutex is locked, false if unlocked.
         */
        if (mutex_is_locked(&_mutex))
        { // The mutex is locked, unlock it.
            mutex_unlock(&_mutex);
        }

    } else {
        pr_alert("%s(): Failed to lock the mutex.\n", __FUNCTION__);
    }

    return res;
}

static void __exit _mutex_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
}

module_init(_mutex_init);
module_exit(_mutex_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Mutual exclusion example.");