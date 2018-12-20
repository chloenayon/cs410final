#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main()
{
    int fd = open("/dev/ttyUSB0", O_RDWR);
    write(fd, "g", 1);
    return 0; 
}
