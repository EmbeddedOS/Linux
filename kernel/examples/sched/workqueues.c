/* workqueues.c - Schedule multiple tasks with the work queues.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <asm/errno.h>

static void _task_f(struct work_struct *data);

static struct workqueue_struct * _workqueue = NULL;
static struct work_struct _work;

static int __init _workqueues_init(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* alloc_workqueue - allocate a workqueue.
     * @fmt: printf format for the name of the workqueue.
     * @flags: WQ_* flags
     * @max_active: max in-flight work items, 0 for default.
     * remaining args: args for @fmt.
     * 
     * Allocate a workqueue with the specified parameters. For
     * detailed information on WQ_* flags, please refer to:
     * https://github.com/torvalds/linux/blob/master/Documentation/core-api/workqueue.rst
     * 
     * ``@flags`` and ``@max_active`` control how work items are assigned 
     * execution resources, scheduled and executed.
     * 
     * ``WQ_UNBOUND``: 
     * 
     * ``@max_active`` determines the maximum number of execution contexts 
     * per CPU which can be assigned to the work items of a wq.
     * 
     * return a pointer to allocated workqueue on success, NULL on failure.
     */
    _workqueue = alloc_workqueue("Larva queue", WQ_UNBOUND, 8);
    if (_workqueue == NULL)
    {
         return -ENOMEM;
    }

    INIT_WORK(&_work, _task_f);

    schedule_work(&_work);

    return 0;
}

static void __exit _workqueues_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    destroy_workqueue(_workqueue);
}

static void _task_f(struct work_struct *data)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
}

module_init(_workqueues_init);
module_exit(_workqueues_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Add multiple task to the work queue.");
