 /* intrpt.c - add a interrupt handler to a interrupt line.
  * check me: cat /dev/larva_dev
  */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

#define DEVICE_NAME "larva_dev" /* Dev name as it appears in /proc/devices */

static int _open(struct inode * inode, struct file *f);
static int _release(struct inode * inode, struct file *f);
static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset);
static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset);

static void _interrupt_task(struct tasklet_struct* data);

/* @name: task name
 * @func: call back function.
 * @data: data argument.
 */
static DECLARE_TASKLET(_task, _interrupt_task);

static unsigned long int_no = 1;
static struct file_operations _fops = {
    .owner = THIS_MODULE,
    .read = _read,
    .write = _write,
    .open = _open,
    .release = _release,
};

/* interrupt handler prototype:
 * irqreturn_t (*irq_handler_t)(int, void *);
 */
irqreturn_t _interrupt_handler(int irq, void *data);

static int _major; /* major number assigned to our device driver */
static struct class *_cls;

static int __init _int_init(void)
{
    int res = 0;
    pr_info("%s(): invoked.\n", __FUNCTION__);

    if (!irq_percpu_is_enabled(int_no))
    {
        pr_alert("IRQ line %lu is not enabled.\n", int_no);
        //res = -EIO;
        //goto out_1;
    }

    _major = register_chrdev(0, DEVICE_NAME, &_fops);
    if (_major < 0)
    { // Registration failed.
        pr_alert("Registering char device failed with %d\n", _major);
        res = _major;
        goto out_1;
    }

    _cls = class_create(THIS_MODULE, DEVICE_NAME);

    device_create(_cls, NULL, MKDEV(_major, 0), NULL, DEVICE_NAME);

    pr_info("Device created on /dev/%s.\n", DEVICE_NAME);

    /* request_irq - Add a handler for an interrupt line.
     * @irq: The interrupt line to allocate.
     * @handler: Function to be called when the IRQ occurs.
     *           Primary handler for threaded interrupts.
     *           If NULL, the default primary handler is installed.
     * @flags: Handling flags.
     * @name: Name of the device generating this interrupt.
     * @dev: A cookie passed to the handler function.
     * 
     * IRQF_SHARED - allow sharing the irq among several devices.
     * 
     * This call allocates an interrupt and establishes a handler;
     * see the documentation for `request_threaded_irq()`for details.
     */
    res = request_irq(int_no, _interrupt_handler, IRQF_SHARED, DEVICE_NAME, (void*)_interrupt_handler);
    if (res)
    {
        pr_info("Unable to request IRQ: %d\n", res);
        goto out_2;
    }

    pr_info("Registered a interrupt handler on interrupt line %lu.\n", int_no);
    return 0;

out_2:
    if (_cls != NULL)
    {
        device_destroy(_cls, MKDEV(_major, 0));
        class_destroy(_cls);
        unregister_chrdev(_major, DEVICE_NAME);
        _cls = NULL;
    }

out_1:
    return res;
}

static void __exit _int_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* We can not pass NULL,
     * this is a shared interrupt handler.
     */
    free_irq(int_no, (void *)_interrupt_handler);

    if (_cls != NULL)
    {
        device_destroy(_cls, MKDEV(_major, 0));
        class_destroy(_cls);
        unregister_chrdev(_major, DEVICE_NAME);
        _cls = NULL;
    }

    pr_info("Unregister the device on /dev/%s.\n", DEVICE_NAME);
}

irqreturn_t _interrupt_handler(int irq, void *data)
{
    tasklet_schedule(&_task);
    return IRQ_HANDLED;
}

static void _interrupt_task(struct tasklet_struct* data)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
}

static int _open(struct inode * inode, struct file *f)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    return 0;
}
static int _release(struct inode * inode, struct file *f)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    return 0;
}

static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* We make a interrupt with irq 1 when the device file is read.
     */
    pr_info("%s(): make interrupt %lu.\n", __FUNCTION__, int_no);
    asm volatile("int $0x01");
    return 0;
}
static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    return 0;
}


module_init(_int_init);
module_exit(_int_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Request a interrupt handler.");
