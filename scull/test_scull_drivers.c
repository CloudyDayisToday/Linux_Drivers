#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/* DEVICE ERROR DEFINITION */
#define DEVICE_OPEN_FAILED -1
#define DEVICE_CLOSE_FAILED -2

/**
 * @brief Write to a scull device
 * 
 * @param device_name device name
 * @param buf write data buffer
 * @param count number of data bytes
 * @return SUCCESS = 0
 */
int device_write(const char* device_name, char* buf, int count)
{
    printf("Device write operation\n");
    int scull = open(device_name, O_RDWR);
    if (scull < 0)
    {
        printf("Failed to open!\n");
        return DEVICE_OPEN_FAILED;
    } 
    int bytes = write(scull, buf, count);
    int err = close(scull);
    if (err < 0) return DEVICE_CLOSE_FAILED;

    return bytes;
}

/**
 * @brief Read from a scull device
 * 
 * @param device_name device name
 * @param buf read data buffer
 * @param count number of data bytes
 * @return SUCCESS = 0
 */
int device_read(const char* device_name, char* buf, int count)
{
    printf("Device read operation\n");
    int scull = open(device_name, O_RDONLY);
    if (scull < 0) return DEVICE_OPEN_FAILED;
    int bytes = read(scull, buf, count);
    int err = close(scull);
    if (err < 0) return DEVICE_CLOSE_FAILED;

    return bytes;
}

/* Randomly generate write data */
char* random_data(int count)
{
    char* ret = malloc(count*sizeof(char));
    for (int i = 0; i < count; ++i)
    {
        ret[i] = rand() % 10;
    }
    return ret;
}

int main(int argc, char **argv)
{
    int bytes, count, err; 
    
    count = 10;
    char *w_buf = random_data(count);
    char *r_buf = malloc(count*sizeof(char));

    /* Calling write operation */
    err = device_write("/dev/scull0", w_buf, count);
    if (err < 0)
    {
        printf("Device write error code: %d\n", err);
    }

    /* Calling read operation */
    err = device_read("/dev/scull0", r_buf, count);
    if (err < 0)
    {
        printf("Device read error code: %d\n", err);
    }

    for (int i = 0; i < count; ++i)
    {
        printf("Write: %d,  Read: %d\n", w_buf[i],r_buf[i]);
    }

    return 0;
}