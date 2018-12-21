#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char** argv)
{
    int fd = open("/dev/ttyUSB0", O_RDWR);
    write(fd, argv[1], 1);
    return 0; 
}
