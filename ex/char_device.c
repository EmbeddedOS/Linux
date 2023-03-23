/* char_device.c: Create a read-only char device that says 
 * how many times u have read from the dev file.
 */

#define OK 0
struct inode;
struct file;

static int __open(struct inode * inode, struct file *f);

static int __open(struct inode * inode, struct file *f)
{

}
