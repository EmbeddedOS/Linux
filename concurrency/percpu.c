/* percpu.c - make per CPU variable and manage it.
 */

#include <linux/types.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/percpu.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_NAME "percpu-dev"
#define OK 0

static int _open(struct inode * inode, struct file *f);
static int _release(struct inode * inode, struct file *f);
static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset);
static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset);


static struct proc_dir_entry *_proc_file;
DECLARE_PER_CPU(int, _per_cpu_var) = 0;

#ifdef HAVE_PROC_OPS
static struct proc_ops _fops = {
    .proc_read = _read,
    .proc_write = _write,
    .proc_open = _open,
    .proc_release = _release,
};
#else
static struct file_operations _fops = {
    .read = _read,
    .write = _write,
    .open = _open,
    .release = _release,
};
#endif

static int __init _module_init(void)
{
    int res = OK;
    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* Create a file in `/proc`.
     * @name: `proc` file name.
     * @mode: file permissions.
     * @parent: 
     * @proc_fops: file operations structure that link to the `proc` file.
     * 
     * The return value is a struct proc_dir_entry, and it will be used to
     * configure the `/proc` file. A NULL return value means that the creation has failed.
     */
    _proc_file = proc_create(PROCFS_NAME, 0644, NULL, &_fops);
    if (_proc_file == NULL)
    {
        pr_alert("ERROR: failed to create /proc/%s file.\n", PROCFS_NAME);
        res = -ENOMEM;
        goto out;
    }

    /* Set the `/proc` file size.*/
    proc_set_size(_proc_file, 100);

    /* Set the `/proc` file owner as root.*/
    proc_set_user(_proc_file, GLOBAL_ROOT_UID, GLOBAL_ROOT_GID);

    pr_info("/proc/%s created.\n", PROCFS_NAME);

out:
    return OK;
}

static void __exit _module_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    proc_remove(_proc_file);
}

static int _open(struct inode * inode, struct file *f)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    try_module_get(THIS_MODULE);

    /* get_cpu_var() return lvalue of the per cpu variable.
     */
    pr_info("%s(): value of per cpu variable is %d on cpu %d.\n", 
                    __FUNCTION__,
                    get_cpu_var(_per_cpu_var)++,
                    smp_processor_id());

    put_cpu_var(_per_cpu_var);
    return OK;
}

static int _release(struct inode * inode, struct file *f)
{
    int i = 0;

    pr_info("%s(): invoked.\n", __FUNCTION__);

    for (i = 0; i < num_online_cpus(); i++)
    {
        pr_info("%s(): Value: %d on cpu: %d.\n", __FUNCTION__, per_cpu(_per_cpu_var, i), i);
    }

    // Or similar with for_each_online_cpu() macro
    i = 0;
    for_each_online_cpu(i)
    {
        pr_info("%s(): Value: %d on cpu: %d.\n", __FUNCTION__, per_cpu(_per_cpu_var, i), i);
    }

    module_put(THIS_MODULE);

    return OK;
}

static ssize_t _read(struct file *f, char __user *p, size_t size, loff_t *offset)
{
    return 0;
}

static ssize_t _write(struct file *f, const char __user *p, size_t size, loff_t *offset)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    return 0;
}

module_init(_module_init);
module_exit(_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Concurrency: per CPU variables.");