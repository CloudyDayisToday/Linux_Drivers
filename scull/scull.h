/*******************************************************************/
/*  This is a char driver program that runs on linux kernel        */
/*  To run the program: sudo insmod scull.ko                       */
/*  To quit the progarm: sudo rmmod scull                          */
/*  When the program is properly loaded,                           */
/*  to find the devices, run: cat /proc/devices                    */
/*******************************************************************/

#pragma once

#include <linux/types.h> // for device type
#include <linux/cdev.h>
#include <linux/semaphore.h>

#define DATA_SIZE 1024
#define SCULL_DEBUG

/*-----------------------------------------DEBUGGING INFO-------*/
#undef PDEBUG /* udef it, just in case*/
#ifdef SCULL_DEBUG
#   ifdef __KERNEL__
        /* THis one if debugging is on, and kernel space */
#       define PDEBUG(fmt, args...) printk(KERN_DEBUG "scull: " fmt, ## args)
#   else 
        /* This one is for user space */
#       define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#   endif
#else
#   define PDEBUG(fmt, args...) /* not debugging: nothing*/
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a place holder */


/*-----------------------------------------STRUCT DEFINITION-------*/
/* Define the struct for a scull device */
struct scull_dev
{
    struct scull_qset *data; /* Pointer to first quantum set */
    int quantum; /* the current quantum size*/
    int qset; /* the current array size */
    unsigned long size; /* amount of data stored here */
    unsigned int access_key; /* used by sculluid and scullpriv */
    struct semaphore sem; /* mutex semaphore */
    struct cdev cdev; /* Char device structure */
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
static void scull_exit (void);

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
 * @return SUCCESS = 0, FAIL = 1
 */
int scull_release(struct inode *inode, struct file *filp);

/**
 * @brief Read a file
 * 
 * @param filp file pointer
 * @param buff a read buffer
 * @param count word count
 * @param offp where it is read
 * @return ssize_t the count of bytes successfully transferred
 */
ssize_t scull_read(struct file *filp, char __user *buff, size_t count, loff_t *offp);


/**
 * @brief Read a file
 * 
 * @param filp file pointer
 * @param buff a write buffer
 * @param count word count
 * @param offp where it is going to write
 * @return ssize_t the count of bytes successfully transferred
 */
ssize_t scull_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);

/**
 * @brief Set up a scull device
 * 
 * @param dev a scull device
 * @param index the device number
 */
static void scull_setup_cdev (struct scull_dev *dev, int index);

/**
 * @brief Follow the list
 * 
 * @param dev scull device
 * @param n position
 * @return struct scull_qset* linked list
 */
struct scull_qset *scull_follow(struct scull_dev *dev, int n);

/*--------------------------------------------SEQ_FILE INTERFACE------*/
int scull_read_procmem(char *buf, char **start, off_t offset, int count, int *eof, void *data);
static void *scull_seq_start(struct seq_file *s, loff_t *pos);
static void *scull_seq_next(struct seq_file *s, void *v, loff_t *pos);
static void scull_seq_stop(struct seq_file *s, void *v);
static int scull_seq_show(struct seq_file *s, void *v);
static int scull_proc_open(struct inode *inode, struct file *file);
static void scull_create_proc(void);
static void scull_remove_proc(void);