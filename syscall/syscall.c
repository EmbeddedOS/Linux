
/* syscall.c - system calls spying.
 * We can get all information of communication between user program
 * and kernel by replacing system calls.
 *
 * For system call number, lookup: `/lib/modules/$(uname -r)/build/include/uapi/asm-generic/unistd.h`
 * For system call prototype, lookup: `/lib/modules/(uname -r)/build/include/linux/syscalls.h`
 * For system call table address, look up: `/boot/System.map-$(uname -r)` or `/proc/kallsyms`.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/moduleparam.h>
#include <asm/errno.h>

#include <linux/cred.h> /* For current_uid(). */

#define OK 0
typedef unsigned long _reg_t; /* 64 bit architecture. */

/* The way we access `sys_call_table` varies as kernel internal changes.
 * - Prior to v5.4: manual symbol lookup.
 * - v5.5 to v5.6 : use kallsyms_lookup_name().
 * - v5.7+        : Kprobes (if any) or specific module parameter.
 */
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 4, 0)
#include <linux/syscalls.h> > /* For some syscall function pointer. */
#define USING_MANUAL_SYMBOL_LOOK_UP

#elif (LINUX_VERSION_CODE > KERNEL_VERSION(5, 4, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0))
#include <linux/kallsyms.h> /* For kallsyms_lookup_name */
#define USING_KALLSYMS_LOOKUP_NAME

#else

#if defined(CONFIG_KPROBES)
#include <linux/kprobes.h>
#define USING_KPROBES

#else
#include <linux/kallsyms.h> /* For sprint_symbol */

/* When the kernel version is v5.7+ without CONFIG_KPROBES,
 * we can obtain the address of  the `sys_call_table` with
 * look up `/boot/System.map-$(uname -r)` or `/proc/kallsyms`.
 */
static _reg_t _sym = 0;
module_param(_sym, ulong, 0644);
#define USING_MODULE_PARAM

#endif

#endif

/* Since Linux v5.3, the write_cr0() function can not be used
 * because of the sensitive `cr0` bits pinned by the security issue,
 * The attacker may write into CPU control registers to disable protections.
 * So, As a result, we have provide the custom assembly routine to bypass it.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0)
static inline void _write_cr0_register(_reg_t cr0)
{
    asm volatile("mov %0,%%cr0" : "+r"(cr0) : : "memory");
}
#else
#define _write_cr0_register write_cr0
#endif


/* Pointer to the original system calls. We should keep this rather 
 * than call the original function, is because somebody else might have
 * replaced the system call before us (-_-). Another reason is that we can
 * not get sys_openat() function because it is a static function and
 * is not exported.
 */
#if defined(CONFIG_ARCH_HAS_SYSCALL_WRAPPER)
static asmlinkage long (*_original_openat)(const struct pt_regs *);

static asmlinkage long _wrapper_openat(const struct pt_regs *);
#else
static asmlinkage long (*_original_openat)(int, const char __user *, int, umode_t);

static asmlinkage long _wrapper_openat(int, const char __user *, int, umode_t);
#endif


static _reg_t **_sys_call_table = NULL;

static void _enable_mm_write_protection(void);
static void _disable_mm_write_protection(void);
static _reg_t** _acquire_syscall_table(void);
static inline unsigned long _get_args(int arg, const struct pt_regs *regs);

static int __init _module_init(void)
{
    int res = OK;

    /* Try to get address of `sys_call_table`. */
    _sys_call_table = _acquire_syscall_table();
    if (_sys_call_table == NULL)
    {
        pr_alert("Failed to look ip system call table.\n");
        res = -EPERM;
        goto out;
    }

    /* Because system call table address located on read-only memory (text section),
     * we need to change control register to CPU can write to this memory. Disables page 
     * protection at a processor level by changing the 16th bit in the cr0 register 
     * (could be Intel specific).
     */
    _disable_mm_write_protection();

    /* Cache the origin system calls, and
     * replace them with our functions.
     */
    _original_openat = (void *)_sys_call_table[__NR_openat];
    _sys_call_table[__NR_openat] = (_reg_t *)_wrapper_openat;

    _enable_mm_write_protection();

out:
    return res;
}

static void __exit _module_exit(void)
{
    if (_sys_call_table == NULL)
    {
        return;
    }

    if ( _sys_call_table[__NR_openat] != (_reg_t *)_wrapper_openat)
    {
        pr_alert("Someone is also replaced the system calls, the system may be left in an unstable state.\n");
    }

    _disable_mm_write_protection();

    /* Return the system call table back to normal.
     */
    _sys_call_table[__NR_openat] = (_reg_t *)_original_openat;

    _enable_mm_write_protection();
}

static void _enable_mm_write_protection(void)
{
    _reg_t cr0 = read_cr0();
    set_bit(16, &cr0); /* Set bit 16: write protect. */
    _write_cr0_register(cr0);
}

static void _disable_mm_write_protection(void)
{
    _reg_t cr0 = read_cr0();
    clear_bit(16, &cr0); /* Clear bit 16: write protect. */
    _write_cr0_register(cr0);
}

static _reg_t **_acquire_syscall_table(void)
{
#if defined(USING_MANUAL_SYMBOL_LOOK_UP)
    _reg_t offset = PAGE_OFFSET;
    _reg_t **table_address;
    while (offset < ULLONG_MAX)
    { /* We will find the table address across the entire kernel memory page.
       */
        table_address == (_reg_t **)offset;

        if (table_address[__NR_close] == (_reg_t *)ksys_close)
        { /* Using some system call function address to determine the system call table.
           */
            return table_address;
        }

        /* We use it to point to next address register.
         * Size of void pointer on 16 bit Platform is: 2 bytes,
         * on 32 bit : 4 bytes and on 64 bit : 8 bytes.
         */
        offset += sizeof(void *);
    }
    
    return NULL;

#elif defined(USING_KALLSYMS_LOOKUP_NAME)
    return (_reg_t **)kallsyms_lookup_name("sys_call_table");

#elif defined(USING_KPROBES)
    /* Get kallsyms_lookup_name() function address by using Kprobe.
     */

    _reg_t (*kallsyms_lookup_name)(const char *name);
    struct kprobe kp = { .symbol_name = "kallsyms_lookup_name"};

    if (register_kprobe(&kp) < 0)
    {
        return NULL;
    }

    kallsyms_lookup_name = (_reg_t (*)(const char *name))kp.addr;
    unregister_kprobe(&kp);

    return (_reg_t **)kallsyms_lookup_name("sys_call_table");

#elif defined(USING_MODULE_PARAM)
    /* We need to validate the symbol input param.
     */
    if (_sym = 0)
    {
        pr_alert("Please specify `sym` argument for sys_call_table. \
                 Look up `/boot/System.map-$(uname -r)` or `/proc/kallsyms`.\n");
        return NULL;
    }

    /* Look up a kernel symbol and return it in a text buffer.
     */
    char symbol[50] = {0};
    sprint_symbol(symbol, _sym);

    if (strncmp("sys_call_table", symbol, sizeof() - 1) == 0)
    {
        return (_reg_t **)_sym;
    }

    return NULL; /* Invalid symbol param. */

#else
    return NULL;
#endif
}

#if defined(CONFIG_ARCH_HAS_SYSCALL_WRAPPER)
static asmlinkage long _wrapper_openat(const struct pt_regs *regs)
{
    int res = _original_openat(regs);

    pr_info("%s(): (%d)(open)(%d, %ld, %d, %d) is invoked by %d.\n",
                __FUNCTION__,
                res,
                (int)_get_args(0, regs), /* File descriptor. */
                _get_args(1, regs),
                (int)_get_args(2, regs), /* Flags. */
                (int)_get_args(3, regs), /* Mode. */
                current->pid);
    return res;
}

#else
static asmlinkage long _wrapper_openat(int fd, const char __user * filename, int flags, umode_t mode)
{
    int res = _original_openat(fd, filename, flags, mode);

    pr_info("%s(): %d.\n", __FUNCTION__, res);
    return res;
}

#endif

static inline unsigned long _get_args(int arg, const struct pt_regs *regs)
{
    if (regs == NULL)
    {
        return 0;
    }

    switch (arg)
    {
    case 0: /* First argument is stored in DI register. */
        return regs->di;
    case 1:/* Second argument is stored in SI register. */
        return regs->si;
    case 2:
        return regs->dx;
    case 3:
        return regs->r10;
    case 4:
        return regs->r8;
    case 5:
        return regs->r9;

    default:
        return 0;
    }
}


module_init(_module_init);
module_exit(_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("STEALING all system calls.");