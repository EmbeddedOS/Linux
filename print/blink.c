/* blink.c - Blink keyboard leds.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/tty.h>
#include <linux/vt.h> /* For MAX_NR_CONSOLES. */
#include <linux/vt_kern.h> /* For fg_console. */
#include <linux/console_struct.h> /* For vc_cons. */

extern int fg_console; /* Current virtual console. */
extern struct vc vc_cons [MAX_NR_CONSOLES];

static struct tty_driver *_driver = NULL;
static struct timer_list _timer;

void _callback(struct timer_list *timer);

static int __init _blink_init(void)
{

    int i = 0;

    pr_info("%s(): invoked.\n", __FUNCTION__);

    for (i = 0; i < MAX_NR_CONSOLES; i++)
    {
    }

    _driver = vc_cons[fg_console].d->port.tty->driver;
    
    timer_setup(&_timer, _callback, 0);

    _timer.expires = jiffies + HZ / 5;

    return 0;
}


static void __exit _blink_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
}

void _callback(struct timer_list *timer)
{

}


module_init(_blink_init);
module_exit(_blink_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Blink keyboard leds.");
