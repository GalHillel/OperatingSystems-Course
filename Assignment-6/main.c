#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "char_device.h"

#define DEVICE_FILE_PATH "/dev/encrypted_memory"

int main(void)
{
    int fd = open(DEVICE_FILE_PATH, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open the device file");
        return -1;
    }

    int key = 123;
    if (ioctl(fd, IOCTL_SET_KEY, &key) < 0)
    {
        perror("Failed to set the key");
        close(fd);
        return -1;
    }

    char buffer[4096];
    ssize_t bytes_read = read(fd, buffer, 4096);
    if (bytes_read < 0)
    {
        perror("Failed to read from the device");
        close(fd);
        return -1;
    }

    ssize_t bytes_written = write(fd, buffer, bytes_read);
    if (bytes_written < 0)
    {
        perror("Failed to write to the device");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}
