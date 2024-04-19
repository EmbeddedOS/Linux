#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#define BAR_0_LENGTH (0x40)   // 64 byte.
#define REG_OP1                 0x10
#define REG_OP2                 0x14
#define REG_OPCODE              0x18
#define REG_RESULT              0x20
#define REG_ERROR               0x24
#define OPCODE_ADD              0x00
#define OPCODE_MUL              0x01
#define OPCODE_DIV              0x02
#define OPCODE_SUB              0x03
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
    uint32_t *ptr = (uint32_t *)(pci_dev_bar0_base + REG_OP1);
    *ptr = 1;

    ptr = (uint32_t *)(pci_dev_bar0_base + REG_OP2);
    *ptr = 2;

    ptr = (uint32_t *)(pci_dev_bar0_base + REG_OPCODE);
    *ptr = OPCODE_ADD;

    printf("Test add operator: %d \n", *(uint32_t *)(pci_dev_bar0_base + REG_RESULT));

    munmap(pci_dev_bar0_base, BAR_0_LENGTH);
    close(fd);

    return 0;
}