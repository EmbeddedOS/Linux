/* tty.c - Redirect data stream to current `tty` (terminal).
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/tty.h>

static void _print(char * msg);

static int __init _tty_init(void)
{
    _print("_tty_init(): invoked.");
    return 0;
}


static void __exit _tty_exit(void)
{
    _print("_tty_exit(): invoked.");
}

static void _print(char * msg)
{
    /* Try to get the tty for the current task. */
    struct tty_struct* tty = get_current_tty();

    /* If tty is NULL, the current task has no tty u can print to (i.e., if it is a daemon). If so, there is nothing we can do. */
    if (tty != NULL)
    {
        /* Get tty operations: write() so we can write to the tty.
         * It can be used to take a string either from the user's or
         * kernel's memory segment.
         */
        const struct tty_operations *ttyops = tty->driver->ops;
        
        /* write data stream to the tty.
         * @tty: teletype to write.
         * @buf: pointer to message.
         * @count: size of the message.
         */
        (ttyops->write)(tty,
                        msg,
                        strlen(msg));

        /* Write end of line. */
        (ttyops->write)(tty, "\015\012", 2);
    }
}


module_init(_tty_init);
module_exit(_tty_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Redirect data stream to current tty.");
