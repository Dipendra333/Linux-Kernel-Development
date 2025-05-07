#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define MY_IOCTL_MAGIC_NUMBER 'D'
#define IOCTL_SET_SECRET _IOW(MY_IOCTL_MAGIC_NUMBER, 2, char *)
#define IOCTL_GET_SECRET _IOR(MY_IOCTL_MAGIC_NUMBER, 1, char *)
#define BUF_SIZE 60


int main(void) {
    int fd;
    char *message;

    fd = open("/dev/my_device_name0",O_RDWR);
    if (fd < 0) {
        perror("Open");
        return -1;
    }

    message = "Dipendra";

    if (ioctl(fd, IOCTL_SET_SECRET, message))
    {
        perror("IOCTL SET");
        close(fd);
        return -1;
    }

    message = malloc(sizeof(char) * BUF_SIZE);
    if( !message)
    {
        perror("MALLOC");
        close(fd);
        return -1;
    }

    memset(message, 0, BUF_SIZE);

    if(ioctl(fd, IOCTL_GET_SECRET, message))
    {
        perror("IOCTL GET");
        close(fd);
        return -1;
    }

    printf(" Secret From IOCTL GET: %s \n",message);

    close(fd);

    return 0;
}
