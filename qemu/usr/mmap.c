#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define BAR_0_LENGTH (0x40)   // 64 byte.

int main()
{
    int fd = open("/dev/c_pci_dev", O_RDWR);
    if (fd < 0) {
        printf("Cannot open device file\n");
        return -1;
    }

    /* Addr is NULL, then the kernel chooses the (page-aligned)
       address at which to create the mapping. */
    uint8_t *pci_dev_bar0_base = mmap(NULL,
                                      BAR_0_LENGTH,
                                      PROT_READ | PROT_WRITE,
                                      MAP_PRIVATE,
                                      fd,
                                      0);
    if (pci_dev_bar0_base == MAP_FAILED) {
        printf("Failed to map with PCI BAR 0\n");
        close(fd);
        return -1;
    }

    printf("mmap() success!.\n");

    munmap(pci_dev_bar0_base, BAR_0_LENGTH);
    close(fd);

    return 0;
}