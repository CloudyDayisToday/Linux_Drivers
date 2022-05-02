#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int scull; // scull devices
    int bytes, count = 8; 
    char *buf;
    buf = malloc(count*sizeof(char));

    for (int i = 0; i < count; ++i)
    {
        buf[i] = i;
    }


    // Open, read, write, and close scull0 device
    scull = open("/dev/scull0", O_WRONLY);
    printf("Scull device: %d\n",scull);

    bytes = write(scull, buf, 8);
    printf("Number of bytes written: %d\n",bytes);

    // bytes = read(scull, r_buf, count);
    // printf("Number of bytes read: %d\n",bytes);

    close(scull);

    return 0;
}