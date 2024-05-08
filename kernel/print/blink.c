/* blink.c - Blink keyboard leds.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/tty.h>
#include <linux/vt.h> /* For MAX_NR_CONSOLES. */
#include <linux/vt_kern.h> /* For fg_console. */
#include <linux/console_struct.h> /* For vc_cons. */

#define ALL_LEDS_ON 0x07
#define RESTORE_ALL_LEDS 0xFF

extern int fg_console; /* Current virtual console. */
extern struct vc vc_cons [MAX_NR_CONSOLES];
extern unsigned long volatile jiffies;

static struct tty_driver *_driver = NULL;
static struct timer_list _timer;
static unsigned long _led_status = 0;

static inline void _reset_timer(void);
static void _callback(struct timer_list *timer);

static int __init _blink_init(void)
{

    int i = 0;

    pr_info("%s(): invoked.\n", __FUNCTION__);

    for (i = 0; i < MAX_NR_CONSOLES; i++)
    {
        if (vc_cons[i].d == NULL)
        {
            break;
        }

        pr_info("%s(): console[%i/%i] #%i, tty %p\n",
                __FUNCTION__,
                i,
                MAX_NR_CONSOLES,
                vc_cons[i].d->vc_num,
                (void *)vc_cons[i].d->port.tty);
    }

    // Get driver of current virtual console tty.
    _driver = vc_cons[fg_console].d->port.tty->driver;
    
    timer_setup(&_timer, _callback, 0);

    _reset_timer();

    return 0;
}


static void __exit _blink_exit(void)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);
    del_timer(&_timer);
    if (_driver != NULL)
    {
        (_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, RESTORE_ALL_LEDS);
    }
}

static inline void _reset_timer(void)
{
    /* The global variable `jiffies` holds the number of ticks
     * that have occurred since the system booted.
     * On boot, the kernel initializes the variable to zero,
     * and it is incremented by one during each timer interrupt.
     * Thus, because there are HZ timer interrupts in a second,
     * there are HZ jiffies in a second.
     * 
     * The system uptime is therefore jiffies/HZ seconds.
     */
    _timer.expires = jiffies + HZ / 5;
    add_timer(&_timer);
}

/* callback function below blinks the keyboard LED periodically by
 * invoking command `KDSETLED` of ioctl() on the keyboard driver.
 * More on virtual terminal ioctl operations, see file:
 * https://github.com/torvalds/linux/blob/master/drivers/tty/vt/vt_ioctl.c
 */
static void _callback(struct timer_list *timer)
{
    pr_info("%s(): invoked.\n", __FUNCTION__);

    /* Switch LED state. */
    if (_led_status == ALL_LEDS_ON)
    {
        _led_status = RESTORE_ALL_LEDS;
    } else
    {
        _led_status = ALL_LEDS_ON;
    }

    if (_driver != NULL)
    {
        pr_info("%s(): switching led state.\n", __FUNCTION__);
        (_driver->ops->ioctl)(vc_cons[fg_console].d->port.tty, KDSETLED, _led_status);
    }

    _reset_timer();
}


module_init(_blink_init);
module_exit(_blink_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Blink keyboard leds.");
