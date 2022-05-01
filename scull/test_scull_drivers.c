#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    // Open and close four devices to test all the drivers
    for (int i = 0; i < 4; ++i)
    {
        int fd = open("/dev/scull0", O_RDONLY);
        int res = close(fd);
    }

    return 0;
}