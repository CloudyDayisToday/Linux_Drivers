/*******************************************************************/ 
/*  This is a char driver program that runs on linux kernel        */
/*  To run the program: sudo insmod scull.ko                       */
/*  To quit the progarm: sudo rmmod scull                          */
/*  When the program is properly loaded,                           */
/*  to find the devices, run: cat /proc/devices                    */
/*******************************************************************/

#include "scull.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h> // for device type
#include <linux/fs.h> // for obtaining device number
#include <linux/cdev.h>
#include <linux/param.h>
#include <asm/uaccess.h> // access user space memory

MODULE_AUTHOR("Tzu-Yun Chang");
MODULE_LICENSE("DUAL BSD/GPL");

unsigned int scull_major = 0; // dynamic by default
unsigned int scull_minor = 0;
unsigned int scull_nr_devs = 4; //node number default
unsigned int scull_quantum = 4000; //
unsigned int scull_qset = 1000; //


module_param(scull_major, uint, S_IRUGO);
module_param(scull_minor, uint, S_IRUGO);
module_param(scull_nr_devs, uint, S_IRUGO);
module_param(scull_quantum, uint, S_IRUGO);
module_param(scull_qset, uint, S_IRUGO);




int scull_open (struct inode *inode, struct file *filp)
{
    struct scull_dev *dev;

    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;

    if ((filp->f_flags & O_ACCMODE) == O_WRONLY)
    {
        scull_trim(dev);
    }

    return 0;
}

int scull_trim (struct scull_dev *dev)
{
    struct scull_qset *next, *dptr;
    int qset = dev->qset;
    int i;

    for (dptr = dev->data; dptr; dptr = next)
    {
        if (dptr->data)
        {
            for (i = 0; i < qset; ++i)
            {
                kfree(dptr->data[i]);
            }
        }
        kfree(dptr->data);
        dptr->data = NULL;
    }
    dev->size = 0;
    dev->quantum = scull_quantum;
    dev->qset = scull_qset;
    dev->data = NULL;
        
    return 0;
}

int scull_release(struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t scull_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{

}

ssize_t scull_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{

}

struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .llseek = scull_llseek,
    .read = scull_read,
    .write = scull_write,
    .ioctl = scull_ioctl,
    .open = scull_open,
    .release = scull_release
};

static void scull_setup_cdev (struct scull_dev *dev, int index)
{
    int err, devno = MKDEV(scull_major, scull_minor + index);
    cdev_init(&dev->cdev, &scull_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &scull_fops;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
    {
        printk(KERN_NOTICE "Error %d adding scull%d", err, index);
    }
}

static int __init scull_init (void)
{
    dev_t dev;
    int result;

    if (scull_major)
    {
        dev = MKDEV(scull_major, scull_minor);
        result = register_chrdev_region(dev, scull_nr_devs, "scull");
    
    }else
    {
        result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs, "scull");
        scull_major = MAJOR(dev);
    }

    if(result < 0)
    {
        printk(KERN_WARNING "Scull: can't get major %d\n", scull_major);
        return result;
    }

    return 0; // return sucess
}

static void __exit scull_exit (void)
{
    dev_t dev = MKDEV(scull_major, scull_minor);
    unregister_chrdev_region(dev, scull_nr_devs);
}



/* Essential to initialise and deinitialise a module */
module_init(scull_init);
module_exit(scull_exit);