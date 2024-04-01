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

- 2. We also to need our device file `hw/misc/custom_pci_dev.c` compiled and integrated into the qemu build.

    ```bash
    cp custom_pci_dev.c qemu/hw/misc/custom_pci_dev.c
    ```

- 3. Rebuild QEMU.
- 4. Check device is exist:

    ```bash
    ./qemu-system-arm -device help | grep 'c_pci_dev'
    ```

### 3.2. About our device

- Our device is simple enough in the point where it has 4 registers:
  - `op1` and `op2`: holding integer arguments.
  - `opcode`: which determines the operation to occur with `op1` and `op2`.
  - `result`: holds the result of calculation.
  - `error`: error code.
