/* sha256.c - demonstration how to calculate a sha256 hash within kernel module.
 */

#include <linux/module.h>
#include <crypto/internal/hash.h>
#include <linux/printk.h>

/* Overwrite printk. */
#undef pr_fmt
#define pr_fmt(fmt) "%s(): " fmt, __FUNCTION__

#define SHA256_LENGTH 32

static void _show_hash_result(char *text, char *hash_sha256);



static int __init _sha256_init(void)
{
    int res = 0;
    char *plaintext = "Some text demo.";
    char hash[SHA256_LENGTH];
    struct crypto_shash *sha256;
    struct shash_desc *shash;

    pr_info("invoked.\n");

    sha256 = crypto_alloc_shash("sha256", 0, 0);
    if (IS_ERR(sha256))
    {
        pr_err("Failed to allocate sha256 algorithm, enable CONFIG_CRYPTO_SHA256 and try again.\n");
        res = -1;
        goto out0;
    }

    shash = kmalloc(sizeof(struct shash_desc) + crypto_shash_descsize(sha256), GFP_KERNEL);
    if (shash == NULL)
    {
        res = -ENOMEM;
        goto out1;
    }

    shash->tfm = sha256;

    if (crypto_shash_init(shash))
    {
        res = -1;
        goto out2;
    }

    if (crypto_shash_update(shash, plaintext, strlen(plaintext)))
    {
        res = -1;
        goto out2;
    }

    if (crypto_shash_final(shash, hash))
    {
        res = -1;
        goto out2;
    }

    _show_hash_result(plaintext, hash);

out2:
    kfree(shash);

out1:
    crypto_free_shash(sha256);

out0:
    return res;
}

static void __exit _sha256_exit(void)
{
    pr_info("invoked.\n");
}

static void _show_hash_result(char *text, char *hash_sha256)
{
    int i;
    char str[SHA256_LENGTH * 2 + 1];

    pr_info("invoked.\n");
    pr_info("sha256 test for string: \"%s\"\n", text);
    
    for (i = 0; i < SHA256_LENGTH; i++)
    {
        sprintf(&str[i * 2], "%02x", (unsigned char)hash_sha256[i]);
    }
    str[i * 2] = 0;
    
    pr_info("%s\n", str);
}


module_init(_sha256_init);
module_exit(_sha256_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Larva");
MODULE_DESCRIPTION("Sha256 algorithm demonstration.");