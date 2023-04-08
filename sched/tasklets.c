 /* tasklets.c - Schedule for a single task.
  */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/interrupt.h>

static void _task_f(struct tasklet_struct* data);

/* @name: task name
 * @func: call back function.
 * @data: data argument.
 */
static DECLARE_TASKLET(_task, _task_f);

static int __init _tasklet_init(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    tasklet_schedule(&_task);
    return 0;
}

static void __exit _tasklet_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    tasklet_kill(&_task);
}

static void _task_f(struct tasklet_struct* data)
{
    pr_info("%s(): entering.\n", __FUNCTION__);
    mdelay(5000);
    pr_info("%s(): exiting.\n", __FUNCTION__);
}


module_init(_tasklet_init);
module_exit(_tasklet_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Scheduling single task.");
