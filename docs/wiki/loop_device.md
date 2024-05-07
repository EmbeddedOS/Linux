# Loop Device

- In Unix-like OSes, a **loop device**, **vnd**(vnode disk), or **lofi**(loop interface) is a **PSEUDO-DEVICE** that makes a computer file accessible as a block device.

- Before use, a loop device must be connected to an extant file in the file system. The association provides the user with an API that allows the file to be used in place of a block special file. Thus, if the file contains an entire file system, the file may then be mounted as if it were a disk device.

- Files of this kind are often used for CD ISO images and floppy disk images. Mounting a file containing system via such a **loop mount** makes the files within that filesystem accessible. They appear in the mount point directory.

- A loop device may allow some kind of data elaboration during this redirection. For example, the device may be unencrypted version of an encrypted file. In such a case, the file associated with a loop device may be another pseudo-device. This is mostly useful when this device contains an encrypted file system.

## 1. Uses of loop mounting

- After mounting a file that holds a file system, the files in that system can be accessed through the usual file system interface of the OS, without any need for special functionality, such as reading and writing.

## 2. Availability

- Various Unix-like OSes provide the loop device functionality using different names.

- In Linux, device names encoded in the symbol table entries of their corresponding device drivers. The device is called a `loop` device and device nodes are usually named `/dev/loop0`, `/dev/loop1`, etc. They can be created with `makedev` for static directory, dynamically by the facilities of the device file system (`udev`), or directly with `mknod`. The management user interface for the loop device is `losetup`, which is part of the package `util-linux`.

- Sometimes, the loop device is erroneously referred to as **loopback** device, but this term is reserved for a **network device** in OSes. The concept of the `loop` device is distinct.

## 3. Example

- Mounting a file containing a disk image on a directory requires two steps:
  - 1. Association of the file with a loop **device node**.
  - 2. Mounting of the loop device at a mount point directory.

- These two operations can be performed either using two separate commands, or through special flags to the mount command.

```bash
# example.img is regular file containing a file system.
losetup /dev/loop0 example.img
mount /dev/loop0 /home/your/dir
```

- To identify an available loop device for use in the above commands:

```bash
losetup -f
```

- The mount utility is usually capable od handling the entire procedure:

```bash
mount -o loop example.img /home/your/dir
```
