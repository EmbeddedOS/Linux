# QEMU for Linux Kernel developers

## 1. QEMU for Linux Kernel developers

- Basically, QEMU is virtualizer. Used for virtualizing softwares or virtual machines.
- QEMU can emulate almost architectures: ARM, risc-V.

### 1.1. Run Linux kernel & rootfs with QEMU

- Install QEMU: `sudo apt install qemu-system-arm`
- Toolchain -> cross compiler: `wget https://developer.arm.com/-/media/Files/downloads/gnu/13.2.rel1/binrel/arm-gnu-toolchain.tar.xz`
  - After extract we change for shorted name: `mv arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/ arm-gnu-toolchain`

- Busybox -> Rootfs: `wget https://busybox.net/downloads/busybox-1.36.1.tar.bz2`
- Kernel source -> build kernel: `wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.6.22.tar.xz`

### 1.2. Build kernel

- Build kernel:

```bash
cd linux-6.6.22

# Generate config files.
make ARCH=arm CROSS_COMPILE=../arm-gnu-toolchain/bin/arm-none-linux-gnueabihf- defconfig

# Build.
make ARCH=arm CROSS_COMPILE=../arm-gnu-toolchain/bin/arm-none-linux-gnueabihf- -j4
```

- Image file is built into `arch/arm/boot/zImage` directory.

### 1.3. Build busybox

```bash
cd busybox-1.36.1

# Generate config files.
make ARCH=arm CROSS_COMPILE=../arm-gnu-toolchain/bin/arm-none-linux-gnueabihf- defconfig
```

- Change some config manual with menu config:

```bash
# Change config with menu config.
make ARCH=arm CROSS_COMPILE=../arm-gnu-toolchain/bin/arm-none-linux-gnueabihf- menuconfig
```

- Select `Build static binary (no shared libs)`. So we don't need to copy share libraries from toolchain to rootfs.

- Build:

```bash
# Build.
make ARCH=arm CROSS_COMPILE=../arm-gnu-toolchain/bin/arm-none-linux-gnueabihf- -j4
```

- Install to rootfs image:

```bash
make ARCH=arm CROSS_COMPILE=../arm-gnu-toolchain/bin/arm-none-linux-gnueabihf- install
```

### 1.4. Create rootfs

- Create rootfs folder:

```bash
mkdir -p rootfs/{bin,sbin,etc,proc,sys,usr/{bin,sbin}}

# Copy from busybox we just built.
cp -av busybox-1.36.1/_install/ rootfs/
```

- Create symbolic link, it will be init process that is run first:

```bash
cd rootfs
ln -sf bin/busybox init
```

- Create some device file for testing:

```bash
mkdir dev
cd dev
sudo mknod -n 660 mem c 1 1

# For console, etc.
sudo mknod -n 660 tty2 c 4 2
sudo mknod -n 660 tty3 c 4 3
sudo mknod -n 660 tty4 c 4 4
```

- Compress all rootfs:

```bash
cd rootfs
find . print0 | cpio --null -ov --format=newc | gzip -9 > ../rootfs.cpio.gz
```

### 1.4. Run QEMU

- Check all architecture support.

```bash
qemu-system-arm -M help
```

- We run with `virt` architecture, we can using `-append` to add kernel parameters:

```bash
qemu-system-arm -M virt -m 256M -kernel linux-6.6.22/arch/arm/boot/zImage
-initrd rootfs.cpio.gz -append "root=/dev/mem" -nographic
```
