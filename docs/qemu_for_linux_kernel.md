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
cp -av busybox-1.36.1/_install/* rootfs/
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
sudo mknod -m 660 mem c 1 1

# For console, etc.
sudo mknod -m 660 tty2 c 4 2
sudo mknod -m 660 tty3 c 4 3
sudo mknod -m 660 tty4 c 4 4
```

- Create init script to mount our system:

```bash
cd rootfs
mkdir -p etc/init.d/
vim etc/init.d/rcS
chmod -R 777 etc/init.d/rcS
```

- `rcS`:

```bash
# We need sysfs for our PCI bus, to read our PCI device available.
mount -t sysfs none /sys
mount -t proc none /proc
```

- Compress all rootfs:

```bash
cd rootfs
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../rootfs.cpio.gz
```

### 1.4. Run QEMU

- Check all architecture support.

```bash
qemu-system-arm -M help
```

- We run with `virt` architecture, we can using `-append` to add kernel parameters:

```bash
qemu-system-arm -M virt -m 256M -kernel linux-6.6.22/arch/arm/boot/zImage -initrd rootfs.cpio.gz -append "root=/dev/mem" -nographic
```

## 2. Build QEMU from source

- We need to build qemu from source when we want to custom some devices, for example PCI devices.

- Install dependency-recommended packages: [link](https://wiki.qemu.org/Hosts/Linux)

```bash
sudo apt-get install git libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev ninja-build
sudo apt-get install git-email -y
sudo apt-get install libaio-dev libbluetooth-dev libcapstone-dev libbrlapi-dev libbz2-dev -y
sudo apt-get install libcap-ng-dev libcurl4-gnutls-dev libgtk-3-dev -y
sudo apt-get install libibverbs-dev libjpeg8-dev libncurses5-dev libnuma-dev -y
sudo apt-get install librbd-dev librdmacm-dev -y
sudo apt-get install libsasl2-dev libsdl2-dev libseccomp-dev libsnappy-dev libssh-dev -y
sudo apt-get install libvde-dev libvdeplug-dev libvte-2.91-dev libxen-dev liblzo2-dev -y
sudo apt-get install valgrind xfslibs-dev -y
sudo apt-get install libnfs-dev libiscsi-dev -y
```

- Clone source:

```bash
git clone https://github.com/qemu/qemu
cd qemu
git submodule update --init --recursive
```

```bash
mkdir build
cd build

# Add target-list we want to use like ARM, RISC-V, X64, etc.
../configure --target-list=arm-softmmu,arm-linux-user
make -j4
```

- Now we can using our binary to simulate our system:

```bash
./qemu-system-arm -M virt -m 256M -kernel ../../linux-6.6.22/arch/arm/boot/zImage -initrd ../../rootfs.cpio.gz -append "root=/dev/mem" -nographic
```

## 3. Implement a custom QEMU PCI device

### 3.1. The article

- [Article](https://www.linkedin.com/pulse/implementing-custom-qemu-pci-device-nikos-mouzakitis/)
- [QEMU device Source Code](https://github.com/NikosMouzakitis/cpcidev_pci/blob/main/custom_pci_dev.c)
- [Kernel Driver Source Code](https://github.com/NikosMouzakitis/cpcidev_pci/blob/main/custom_qemu_device_driver.c)

- In this article we will be implementing a custom QEMU device, by integrating our device into the qemu build and then creating a driver to access the device functionality from the Linux-side.

- 1. To let QEMU know about our device we need to add configurations:
  - Add device config `hw/misc/Kconfig`:

    ```text
    ## addition for CUSTOM PCI DEVICE
    config C_PCI_DEV
        bool
        default y if TEST_DEVICES
        depends on PCI && MSI_NONBROKEN
    ```

  - Add to build system `hw/misc/meson.build`:

    ```text
    system_ss.add(when: 'CONFIG_C_PCI_DEV', if_true: files('custom_pci_dev.c'))
    ```

  - Or apply the `build.patch` file.
- 2. We also to need our device file `hw/misc/custom_pci_dev.c` compiled and integrated into the qemu build.

    ```bash
    cp custom_pci_dev.c qemu/hw/misc/custom_pci_dev.c
    ```

- 3. Rebuild QEMU.
- 4. Check device is exist:

    ```bash
    ./qemu-system-arm -device help | grep 'c_pci_dev'
    ```

- 5. Run QEMU with our device (NOTE: we need to use new arch `virt-2.10` to support PCI):

    ```bash
    ./qemu-system-arm -M virt-2.10 -kernel ../../linux-6.6.22/arch/arm/boot/zImage -initrd ../../rootfs.cpio.gz -append "root=/dev/mem" -nographic -device c_pci_dev
    ```

  - The log:

    ```text
    [    0.977193] pci 0000:00:02.0: [1234:abcd] type 00 class 0x00ff00
    ```

  - CHeck device exist with `lspci`. The log:

    ```text
    00:00.0 Class 0600: 1b36:0008
    00:02.0 Class 00ff: 1234:abcd
    00:01.0 Class 0200: 1af4:1000
    ```

### 3.2. About our device

- Our device is simple enough in the point where it has 4 registers:
  - `op1` and `op2`: holding integer arguments.
  - `opcode`: which determines the operation to occur with `op1` and `op2`.
  - `result`: holds the result of calculation.
  - `error`: error code.

## 4. Develop some pci utilities (lspci, setpci) for ARM

- Because the busybox support PCI utilities very little, we need to add some utilities.
- 1. Clone repo:

    ```bash
    git clone git://git.kernel.org/pub/scm/utils/pciutils/pciutils.git
    ```

- 2. Apply the `pciutils.patch` to config Makefile.
- 3. Copy binaries `lspci` and `setpci` to our rootfs.

```bash
cd rootfs
cp ../pciutils/lspci usr/bin/c_lspci
cp ../pciutils/setpci usr/bin/c_setpci
```

- 4. Copy dependency libraries to our rootfs.

```bash
# Check needed shared libraries: readelf -d ../pciutils/lspci
cd rootfs
mkdir lib

# lspci depends on these.
cp ../pciutils/lib/libpci.so.3.11.1 lib/libpci.so.3
cp ../arm-gnu-toolchain/arm-none-linux-gnueabihf/libc/lib/libc.so.6 lib/

# libc.so depends on this.
cp ../arm-gnu-toolchain/arm-none-linux-gnueabihf/libc/lib/ld-linux-armhf.so.3 lib/
```

- 5. Rebuild busybox with libc shared libraries, we will add shared libraries by our self so we need to disable `Build static binary (no shared libs)`:

```bash
cd busybox-1.36.1
# Change config with menu config.
make ARCH=arm CROSS_COMPILE=../arm-gnu-toolchain/bin/arm-none-linux-gnueabihf- menuconfig

# Rebuild.
make ARCH=arm CROSS_COMPILE=../arm-gnu-toolchain/bin/arm-none-linux-gnueabihf- -j4

# Recopy.
cp -av _install/* ../rootfs/

```

- Check needed shared libraries of busybox: `readelf -d busybox` we found some dependencies:

```text
 0x00000001 (NEEDED)                     Shared library: [libm.so.6]
 0x00000001 (NEEDED)                     Shared library: [libresolv.so.2]
 0x00000001 (NEEDED)                     Shared library: [libc.so.6]
```

- We need to copy them from the toolchain also:

```bash
cd rootfs

# libc.so depends on this.
cp ../arm-gnu-toolchain/arm-none-linux-gnueabihf/libc/lib/libresolv.so.2 lib/
cp ../arm-gnu-toolchain/arm-none-linux-gnueabihf/libc/lib/libm.so.6 lib/
```

- Re-compress:

```bash
cd rootfs
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../rootfs.cpio.gz
```

- Test our new commands:

```bash
./qemu-system-arm -M virt-2.10 -kernel ../../linux-6.6.22/arch/arm/boot/zImage -initrd ../../rootfs.cpio.gz -append "root=/dev/mem" -nographic -device c_pci_dev

# More info
c_lspci  -s 00:02.0 -vvv
```
