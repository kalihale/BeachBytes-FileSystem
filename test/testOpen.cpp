#include <fcntl.h>  // For open()
#include <unistd.h> // For close()
#include <stdio.h>  // For perror()
#include <errno.h>  // For errno

int main() {
    const char *device = "/dev/vdb"; // Path to the block device
    int fd;

    // Open the block device
    fd = open(device, O_RDONLY);
    if (fd == -1) {
        // Handle errors
        perror("Failed to open block device");
        return 1;
    }

    printf("Block device '%s' opened successfully with file descriptor %d\n", device, fd);

    // Perform operations (read, write, ioctl, etc.)

    // Close the device
    if (close(fd) == -1) {
        perror("Failed to close block device");
        return 1;
    }

    printf("Block device '%s' closed successfully\n", device);
    return 0;
}
