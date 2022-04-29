/*******************************************************************/ 
/*  This is a char driver program that runs on linux kernel        */
/*  To run the program: sudo insmod scull.ko                       */
/*  To quit the progarm: sudo rmmod scull                          */
/*  When the program is properly loaded,                           */
/*  to find the devices, run: cat /proc/devices                    */
/*******************************************************************/

#include <linux/types.h> // for device type
#include <linux/fs.h> // for obtaining device number

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_AUTHOR("Tzu-Yun Chang");
MODULE_LICENSE("DUAL BSD/GPL");

unsigned int scull_major = 0; // dynamic by default
unsigned int scull_minor = 0;
unsigned int scull_nr_devs = 4; //node number default

module_param(scull_major, uint, S_IRUGO);
module_param(scull_minor, uint, S_IRUGO);
module_param(scull_nr_devs, uint, S_IRUGO);

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