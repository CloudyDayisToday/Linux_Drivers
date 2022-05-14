#include "scull.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/types.h> // for device type
#include <linux/fs.h>    // for obtaining device number
#include <linux/cdev.h>
#include <linux/param.h>
#include <asm/uaccess.h> // access user space memory
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/semaphore.h>

MODULE_AUTHOR("Tzu-Yun Chang");
MODULE_LICENSE("DUAL BSD/GPL");

/*
 * Declaring module variables and the module device
 */
unsigned int scull_major = 0; // dynamic by default
unsigned int scull_minor = 0;
unsigned int scull_nr_devs = 4;    // node number default
unsigned int scull_quantum = 4000; 
unsigned int scull_qset = 1000;   
struct scull_dev *scull_devices; 

/*
 * Setting up module parameters for the module variables
 */
module_param(scull_major, uint, S_IRUGO);
module_param(scull_minor, uint, S_IRUGO);
module_param(scull_nr_devs, uint, S_IRUGO);
module_param(scull_quantum, uint, S_IRUGO);
module_param(scull_qset, uint, S_IRUGO);



int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev;

    printk(KERN_INFO "Scull device is being opened\n");

    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;

    if ((filp->f_flags & O_ACCMODE) == O_WRONLY)
    {
        scull_trim(dev);
    }
    return 0;
}

int scull_trim(struct scull_dev *dev)
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
            kfree(dptr->data);
            dptr->data = NULL;
        }
        next = dptr->next;
        kfree(dptr);
    }
    dev->size = 0;
    dev->quantum = scull_quantum;
    dev->qset = scull_qset;
    dev->data = NULL;

    return 0;
}

int scull_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "Scull device is being released\n");
    return 0;
}

struct scull_qset *scull_follow(struct scull_dev *dev, int n)
{
    struct scull_qset *qs = dev->data;

    printk(KERN_INFO "Scull device is being followed\n");

    if (!qs)
    {
        qs = dev->data = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
        if (qs == NULL)
        {
            return NULL;
        }
        memset(qs, 0, sizeof(struct scull_qset));
    }

    while (n--)
    {
        if (!qs->next)
        {
            qs->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
            if (qs->next == NULL)
            {
                return NULL;
            }
            memset(qs, 0, sizeof(struct scull_qset));
        }
        qs = qs->next;
    }
    return qs;
}

ssize_t scull_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr; /* the first listitem */
    int quantum = dev->quantum, qset = dev->qset;
    int itemsize = quantum * qset;
    int item, s_pos, q_pos, rest;
    ssize_t retval = 0;

    printk(KERN_INFO "Scull device is being read\n");

    if (*f_pos >= dev->size)
        goto out;
    if (*f_pos + count > dev->size)
        count = dev->size - *f_pos;

    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    dptr = scull_follow(dev, item);

    if (dptr == NULL || !dptr->data || !dptr->data[s_pos])
        goto out;

    if (count > quantum - q_pos)
        count = quantum - q_pos;
    if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count))
    {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

out:
    return retval;
}

ssize_t scull_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    struct scull_qset *dptr; /* the first listitem */
    int quantum = dev->quantum, qset = dev->qset;
    int itemsize = quantum * qset;
    int item, s_pos, q_pos, rest;
    ssize_t retval = -ENOMEM; /* value used in "goto" statement */

    printk(KERN_INFO "Scull device is being written\n");

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    dptr = scull_follow(dev, item);
    if (dptr == NULL)
        goto out;
    if (!dptr->data)
    {
        dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
        if (!dptr->data)
            goto out;
        memset(dptr->data, 0, qset * sizeof(char *));
    }

    if (!dptr->data[s_pos])
    {
        dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
        if (!dptr->data[s_pos])
            goto out;
    }

    /* write only up to the end of this quantum */
    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_from_user(dptr->data[s_pos] + q_pos, buf, count))
    {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

    if (dev->size < *f_pos)
        dev->size = *f_pos;

out:
    up(&dev->sem);
    return retval;
}

struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .read = scull_read,
    .write = scull_write,
    .open = scull_open,
    .release = scull_release
};

static void scull_setup_cdev(struct scull_dev *dev, int index)
{
    int err, devno = MKDEV(scull_major, scull_minor + index);
    cdev_init(&dev->cdev, &scull_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &scull_fops;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
    {
        printk(KERN_NOTICE "Error %d adding scull%d\n", err, index);
    }
}

static void scull_exit(void)
{
    dev_t dev = MKDEV(scull_major, scull_minor);
    int i;

    printk(KERN_INFO "Scull device is de-initialised\n");

    if (scull_devices)
    {
        for (i = 0; i < scull_nr_devs; ++i)
        {
            scull_trim(scull_devices);
            cdev_del(&scull_devices[i].cdev);
        }
        kfree(scull_devices);
    }

#ifdef SCULL_DEBUG
    scull_remove_proc();
#endif

    unregister_chrdev_region(dev, scull_nr_devs);
}

static int __init scull_init(void)
{
    dev_t dev;
    int result, i;

    printk(KERN_INFO "Scull device is initilised\n");

    if (scull_major)
    {
        dev = MKDEV(scull_major, scull_minor);
        result = register_chrdev_region(dev, scull_nr_devs, "scull");
    }
    else
    {
        result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs, "scull");
        scull_major = MAJOR(dev);
    }

    if (result < 0)
    {
        printk(KERN_WARNING "Scull: can't get major %d\n", scull_major);
        return result;
    }

    scull_devices = kmalloc(scull_nr_devs * sizeof(struct scull_dev), GFP_KERNEL);
    if (!scull_devices)
    {
        result = -ENOMEM;
        goto fail;
    }
    memset(scull_devices, 0, scull_nr_devs * sizeof(struct scull_dev));

    /* Initialise each device */
    for (i = 0; i < scull_nr_devs; ++i)
    {
        scull_devices[i].quantum = scull_quantum;
        scull_devices[i].qset = scull_qset;
        init_MUTEX(&scull_devices[i].sem);
        scull_setup_cdev(&scull_devices[i], i);
    }

#ifdef SCULL_DEBUG
    scull_create_proc();
#endif

    return 0; // return sucess

fail:
    scull_exit();
    return result;
}


int scull_read_procmem(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	int i, j, len = 0;
	int limit = count - 80; /* Don't print more than this */

	for (i = 0; i < scull_nr_devs && len <= limit; i++) 
    {
		struct scull_dev *d = &scull_devices[i];
		struct scull_qset *qs = d->data;

		len += sprintf(buf+len,"\nDevice %i: qset %i, q %i, sz %li\n",
				i, d->qset, d->quantum, d->size);
		for (; qs && len <= limit; qs = qs->next) /* scan the list */
		{
        	len += sprintf(buf + len, "  item at %p, qset at %p\n",
					qs, qs->data);
			if (qs->data && !qs->next) /* dump only the last item */
            {
				for (j = 0; j < d->qset; j++)
                {
					if (qs->data[j])
						len += sprintf(buf + len,
								"    % 4i: %8p\n",
								j, qs->data[j]);
				}
            }
		}
		up(&scull_devices[i].sem);
	}
	*eof = 1;
	return len;
}

static void *scull_seq_start(struct seq_file *s, loff_t *pos)
{
    if (*pos >= scull_nr_devs)
    {
        return NULL;
    }
    return scull_devices + *pos;
}

static void *scull_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
    (*pos)++;
    if (*pos >= scull_nr_devs)
    {
        return NULL;
    }
    return scull_devices + *pos;
}

static void scull_seq_stop(struct seq_file *s, void *v)
{

}

static int scull_seq_show(struct seq_file *s, void *v)
{
    struct scull_dev *dev = (struct scull_dev *) v;
    struct scull_qset *d;
    int i;

    seq_printf(s, "\nDevice %i:  qset %i, q %i, sz %li\n", 
            (int)(dev - scull_devices), dev->qset, dev->quantum, dev->size);
    
    for (d = dev->data; d; d = d->next)
    {
        /* Scan the list */
        seq_printf(s, "item at %p, qset at %p\n", d, d->data);
        if (d->data && !d->next)
        {
            for (i = 0; i < dev->qset; i++)
            {
                if (d->data[i])
                {
                    seq_printf(s, "   %4i: %8p\n", i, d->data[i]);
                }
            }
        }
    }
    return 0;
}

static struct seq_operations scull_seq_ops = {
    .start = scull_seq_start,
    .next = scull_seq_next,
    .stop = scull_seq_stop,
    .show = scull_seq_show
};

static int scull_proc_open(struct inode *inode, struct file *file)
{
    return seq_open(file, &scull_seq_ops);
}

static const struct proc_ops scull_proc_ops = {
    .proc_open = scull_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = seq_release
};

static void scull_create_proc(void)
{
    struct proc_dir_entry *entry;

    entry = proc_create("scull_seq", 0, NULL, &scull_proc_ops);
    if (entry)
    {
        printk(KERN_INFO "Cannot create scull proc\n");
    }
}

static void scull_remove_proc(void)
{
    remove_proc_entry("scullmem", NULL);
    remove_proc_entry("scullseq", NULL);
}


/* Essential to initialise and deinitialise a module */
module_init(scull_init);
module_exit(scull_exit);