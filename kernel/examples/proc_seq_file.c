/* proc.c - Create a file in `/proc` and manage it.
 */

#include <linux/types.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h> /* For seq_file. */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_NAME "larva-dev"
#define OK 0

static int _open(struct inode * inode, struct file *f);
static void *_seq_start(struct seq_file *s, loff_t *pos);
static void *_seq_next(struct seq_file *s, void* v, loff_t *pos);
static void _seq_stop(struct seq_file *s, void* v);
static int _seq_show(struct seq_file *s, void *v);

static struct proc_dir_entry *_proc_file;

static int counter = 0;

static struct seq_operations _seq_ops = {
    .start = _seq_start,
    .next = _seq_next,
    .stop = _seq_stop,
    .show = _seq_show
};

#ifdef HAVE_PROC_OPS
static struct proc_ops _fops = {
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_open = _open,
    .proc_release = seq_release
};
#else
static struct file_operations _fops = {
    .read = seq_read,
    .lseek = seq_lseek,
    .open = _open,
    .release = seq_release,
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

    return seq_open(f, &_seq_ops);
}

/* This callback is called at the beginning of a sequence.
 * - When:
 * + The `/proc` file is read (first time).
 * + After the function stop (end of sequence).
 */
static void *_seq_start(struct seq_file *s, loff_t *pos)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    
    if (*pos == 0)
    { // Begin a new sequence.
        return &counter;
    }

    // It is the end of the sequence, return NULL to stop reading.
    *pos = 0;
    return NULL;
}

/* This function is called after the beginning of a sequence.
 * It is called until the return is NULL.
 */
static void *_seq_next(struct seq_file *s, void* v, loff_t *pos)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    int* temp_pointer_to_my_counter = (int *)v;

    (*temp_pointer_to_my_counter)++;
    (*pos)++;

    pr_info("next() is called %d times.\n", *temp_pointer_to_my_counter);

    if (*temp_pointer_to_my_counter >= 5)
    { // We only run next() 5 times.
        *temp_pointer_to_my_counter = 0;
        return NULL;
    }

    return v;
}

static void _seq_stop(struct seq_file *s, void* v)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    // Do nothing.
}

/* This function is called each `step` in sequence.
 */
static int _seq_show(struct seq_file *s, void *v)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    pr_info("Current step is: %d.\n", *(int *)v);

    return OK;
}

module_init(_module_init);
module_exit(_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Kernel module that create a file in `/proc`.");