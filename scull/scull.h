#pragma once

/*-----------------------------------------STRUCT DEFINITION-------*/

/* Define the struct for a scull device */
struct scull_dev
{
    struct scull_qset *data;
    int quantum;
    int qset;
    unsigned long size;
    unsigned int access_key;
    struct semaphore sem;
    struct cdev cdev;
};

struct scull_qset
{
    void **data;
    struct scull_qset *next;
};

/*-----------------------------------------PUBLIC FUNCTION DECLARATION-------*/

/**
 * @brief Initialise a scull driver
 * 
 * @return SUCCESS = 0, FAIL = -1
 */
static int __init scull_init (void);

/**
 * @brief Exit a scull driver
 * 
 */
static void __exit scull_exit (void);

/**
 * @brief Open a scull driver
 * 
 * @param inode 
 * @param filp a file point
 * @return SUCCESS = 0
 */
int scull_open (struct inode *inode, struct file *filp);

/**
 * @brief Delete the content in a file
 * 
 * @param dev a scull device
 * @return SUCCESS = 0
 */
int scull_trim (struct scull_dev *dev);

/**
 * @brief Empty all the memory related to a scull device
 * 
 * @param inode 
 * @param filp a file pointer
 * @return int SUCCESS = 0
 */
int scull_release(struct inode *inode, struct file *filp);

/**
 * @brief Read a file
 * 
 * @param filp file pointer
 * @param buff a read buffer
 * @param count word count
 * @param offp where it is read
 * @return ssize_t to check if the read size is correct
 */
ssize_t scull_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);


/**
 * @brief Read a file
 * 
 * @param filp file pointer
 * @param buff a write buffer
 * @param count word count
 * @param offp where it is going to write
 * @return ssize_t to check if the write size is correct
 */
ssize_t scull_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);

/**
 * @brief Set up a scull device
 * 
 * @param dev a scull device
 * @param index the device number
 */
static void scull_setup_cdev (struct scull_dev *dev, int index);