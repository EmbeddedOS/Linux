/* completions.c - Start two thread, but one needs to start before another.
 */

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/completion.h>
#include <linux/kthread.h>
#include <linux/err.h>  /* For IS_ERR(). */
#include <linux/delay.h>

struct completion _comp_1;
struct completion _comp_2;

int _thread_1_f(void *unused);
int _thread_2_f(void *unused);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 17, 0)
#define _complete_and_exit(arg1, arg2) kthread_complete_and_exit(arg1, arg2)
#else
#define _complete_and_exit(arg1, arg2) complete_and_exit(arg1, arg2)
#endif

static int __init _completion_init(void)
{
    int res = 0;
    struct task_struct* task_1;
    struct task_struct* task_2;

    pr_info("%s(): invoked.\n", __FUNCTION__);

    init_completion(&_comp_1);
    init_completion(&_comp_2);

    /* kthread_create - create a kthread on the current node.
     * @threadfn: the function to run in the thread.
     * @data: data pointer for @threadfn()
     * @namefmt: printf-style format string for the thread name.
     * @arg: arguments for @namefmt.
     * 
     * This macro will create a kthread on the current node,
     * leaving it in the `stopped` stop. This is a helper for
     * kthread_create_on_node();
     */
    task_1 = kthread_create(_thread_1_f, NULL, "thread 1");
    if (IS_ERR(task_1))
    {
        res = PTR_ERR(task_1);
        goto out;
    }

    task_2 = kthread_create(_thread_2_f, NULL, "thread 2");
    if (IS_ERR(task_2))
    {
        res = PTR_ERR(task_2);
        goto out_1;
    }

    /* wake_up_process - Wake up a specific process.
     * @p: The process to woken up.
     * Attempt to wake up the nominated process 
     * and move it to the set of runnable processes.
     * 
     * Return 1 if the process was woken up, 0 if
     * it was already running.
     * 
     * This function executes a full memory barrier
     * (https://en.wikipedia.org/wiki/Memory_barrier)
     * before accessing the task state.
     */

    wake_up_process(task_1);
    wake_up_process(task_2);

    return res;

out_1:
    kthread_stop(task_2);
out:
    return res;
}

static void __exit _completion_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* wait_for_completion - waits for completion of a task.
     * @x: holds the state of this particular completion.
     * 
     * This waits to be signaled for completion of a specific task.
     * It is NOT interruptible anf there is no timeout.
     * See also similar routines wait_for_completion_timeout() with
     * timeout and interrupt capability. Also see complete().
     */
    wait_for_completion(&_comp_1);
    wait_for_completion(&_comp_2);
}

int _thread_1_f(void *unused)
{
    /* Waiting for thread 2 completely
     * and then continue.
     */
    wait_for_completion(&_comp_2);

    pr_info("%s(): start executing.\n", __FUNCTION__);

    complete_all(&_comp_1);

    _complete_and_exit(&_comp_1, 0);
}

int _thread_2_f(void *unused)
{
    pr_info("%s(): start executing.\n", __FUNCTION__);
    mdelay(5000);

    /* complete_all - signals all threads waiting on this completion.
     * @x: holds the state of this particular completion.
     * 
     * This will wake up all threads waiting on this particular completion event.
     * If this function wakes up a task it executes a full memory barrier
     * before accessing the task state.
     * 
     * Since complete_all() sets the completion of @x permanently to done to allow
     * multiple waiters to finish, a call `reinit_completion()` must be used on @x
     * if @x is to be used again.
     */
    complete_all(&_comp_2);

    _complete_and_exit(&_comp_2, 0);
}

module_init(_completion_init);
module_exit(_completion_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Completions example.");