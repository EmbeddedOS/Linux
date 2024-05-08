#include <stdlib.h>
#include <stdio.h> /* standard I/O */
#include <fcntl.h> /* open */
#include <unistd.h> /* close */
#include <stdlib.h> /* exit */
#include <sys/ioctl.h> /* ioctl */
#include <errno.h>

// Define explanation in the ioctl kernel module.
#define LAVA_MAGIC_NUMBER '\x66'
#define IOCTL_GET_NUM _IOR(LAVA_MAGIC_NUMBER, 1, int)
#define IOCTL_SET_NUM _IOW(LAVA_MAGIC_NUMBER, 2, int)

#define DEVICE_PATH "/dev/larva-ioctl"

int main()
{
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0)
    {
        printf("Failed to open device file: %d.\n", errno);
        exit(EXIT_FAILURE);
    }

    int value = 23111999;

    int res = ioctl(fd, IOCTL_SET_NUM, &value);
    if (res < 0)
    {
        printf("Failed to set private data: %d.\n", errno);
        exit(EXIT_FAILURE);
    }

    printf("run command IOCTL_SET_NUM successfully.\n");

    res = ioctl(fd, IOCTL_GET_NUM, &value);
    if (res < 0)
    {
        printf("Failed to get private data: %d.\n", errno);
        exit(EXIT_FAILURE);
    }

    printf("run command IOCTL_GET_NUM with result: %d.\n", value);

    exit(EXIT_SUCCESS);
}
