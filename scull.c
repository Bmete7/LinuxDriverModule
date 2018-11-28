#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <asm/switch_to.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */

#include "scull_ioctl.h"

#define SCULL_MAJOR 0
#define SCULL_NR_DEVS 4
#define SCULL_QUANTUM 4000
#define SCULL_QSET 1000

int scull_major = SCULL_MAJOR;
int scull_minor = 0;
int scull_nr_devs = SCULL_NR_DEVS;
int scull_quantum = SCULL_QUANTUM;
int scull_qset = SCULL_QSET;

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);
module_param(scull_quantum, int, S_IRUGO);
module_param(scull_qset, int, S_IRUGO);

MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");

struct message{
	char* text;
	struct message* next;
	struct message* prev;
};


struct scull_dev {
    char **data;
    struct message* head;
    struct message* tail;
    int quantum;
    int qset;
    unsigned long size;
    struct semaphore sem;
    struct cdev cdev;
};

struct scull_dev *scull_devices;


int scull_trim(struct scull_dev *dev)
{
    int i;
	
    if (dev->data) {
        for (i = 0; i < dev->qset; i++) {
            if (dev->data[i])
                kfree(dev->data[i]);
        }
        kfree(dev->data);
    }
    dev->data = NULL;
    dev->quantum = scull_quantum;
    dev->qset = scull_qset;
    dev->size = 0;
    return 0;
}


int scull_open(struct inode *inode, struct file *filp)
{
    struct scull_dev *dev;

    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;

    /* trim the device if open was write-only */
    if ((filp->f_flags & O_ACCMODE) == O_WRONLY) {
        if (down_interruptible(&dev->sem))
            return -ERESTARTSYS;
        scull_trim(dev);
        up(&dev->sem);
    }
    return 0;
}


int scull_release(struct inode *inode, struct file *filp)
{
	
    return 0;
}


ssize_t scull_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos)
{
    struct scull_dev *dev = filp->private_data;
    if(dev == scull_devices){
		return -EACCES;
	}
    int quantum = dev->quantum;
    int s_pos, q_pos;
    ssize_t retval = 0;
    
	struct message *tmpMsg= dev->head;
	//memset(tmpMsg, 0, sizeof(struct message));
	
	int wordLength = 0;
	
	
	while(tmpMsg){
		
		wordLength += (strlen(tmpMsg->text));
		//printk("Element = %s\n",tmpMsg->text);
		tmpMsg = tmpMsg->next;	
	}
	
	char* temp = (char*) kmalloc((wordLength+2)*sizeof(char), GFP_KERNEL);
	

	tmpMsg = dev->head;
	wordLength  = 0;
	while(tmpMsg){
		int len= (strlen(tmpMsg->text));
		wordLength += len;
		if(tmpMsg == dev->head){
			strcpy(temp,tmpMsg->text);
		}
		else{					

			strcat(temp,tmpMsg->text);
		}
		tmpMsg = tmpMsg->next;	
		printk("Final: %s\n", temp);
	}
//	strcat(temp,"\n\0");
	temp[wordLength] = '\n';
	temp[wordLength+1] = '\0';
	//printk("%d - %s ", wordLength, temp);
	
	
	
    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;
    if (copy_to_user(buf, temp, wordLength+2)) {
        retval = -EFAULT;
        goto out;
    }
	
	
	kfree(temp);

    if (*f_pos >= dev->size)
        goto out;
    if (*f_pos + count > dev->size)
        count = dev->size - *f_pos;

    s_pos = (long) *f_pos / quantum;
    q_pos = (long) *f_pos % quantum;

    if (dev->data == NULL || ! dev->data[s_pos])
        goto out;

    /* read only up to the end of this quantum */
    if (count > quantum - q_pos)
        count = quantum - q_pos;

    /*if (copy_to_user(buf, dev->data[s_pos] + q_pos, count)) {
        retval = -EFAULT;
        goto out;
    }*/
    *f_pos += count;
    retval = wordLength+2;// count idi degistir

  out:
    up(&dev->sem);
    return retval;
}


ssize_t scull_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos)
{
	
	int quantum,qset;
    struct scull_dev *dev = filp->private_data;
    if(dev == scull_devices){
		return -EACCES;
	}
    char* temp = kmalloc(count*sizeof(char), GFP_KERNEL);
    memset(temp, 0, count*sizeof(char));
	copy_from_user(temp,buf,count);
	temp[count-1] = '\0';
	struct message *tmpMsg= kmalloc(sizeof(struct message), GFP_KERNEL);
	memset(tmpMsg, 0, sizeof(struct message));
	tmpMsg->text = temp;
	tmpMsg->next = NULL;
	tmpMsg->prev = NULL;
    if(!dev->head){
		dev->head = tmpMsg;
		dev->tail = tmpMsg;
	}
	else{
		tmpMsg->prev = dev->tail;
		dev->tail->next = tmpMsg;
		dev->tail = tmpMsg;
	}
    printk("inside struct: %s\n", dev->head->text);
    
    quantum = dev->quantum;
    qset = dev->qset;
    int s_pos, q_pos;
    ssize_t retval = -ENOMEM;

    if (down_interruptible(&dev->sem))
        return -ERESTARTSYS;

    if (*f_pos >= quantum * qset) {
        retval = 0;
        goto out;
    }

    s_pos = (long) *f_pos / quantum;
    q_pos = (long) *f_pos % quantum;

    if (!dev->data) {
        dev->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
        if (!dev->data)
            goto out;
        memset(dev->data, 0, qset * sizeof(char *));
    }
    if (!dev->data[s_pos]) {
        dev->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
        if (!dev->data[s_pos])
            goto out;
    }
    /* write only up to the end of this quantum */
    if (count > quantum - q_pos)
        count = quantum - q_pos;
	
    if (copy_from_user(dev->data[s_pos] + q_pos, buf, count)) {
        retval = -EFAULT;
        goto out;
    }
    *f_pos += count;
    retval = count;

    /* update the size */
    if (dev->size < *f_pos)
        dev->size = *f_pos;
	
  out:
    up(&dev->sem);
    return retval;
}

long scull_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int i = 0;
	int err = 0, tmp;
	int retval = 0;

	/*
	 * extract the type and number bitfields, and don't decode
	 * wrong cmds: return ENOTTY (inappropriate ioctl) before access_ok()
	 */
	if (_IOC_TYPE(cmd) != SCULL_IOC_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > SCULL_IOC_MAXNR) return -ENOTTY;

	/*
	 * the direction is a bitmask, and VERIFY_WRITE catches R/W
	 * transfers. `Type' is user-oriented, while
	 * access_ok is kernel-oriented, so the concept of "read" and
	 * "write" is reversed
	 */
	if (_IOC_DIR(cmd) & _IOC_READ)
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
	else if (_IOC_DIR(cmd) & _IOC_WRITE)
		err =  !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
	if (err) return -EFAULT;

	switch(cmd) {
	  case SCULL_IOCQPOP:
		
		
		for (i = 0 ; i < 3 ; ++i){
			if(scull_devices+1+1){
				if((scull_devices+1+i)->head){

					struct scull_dev *dev = filp->private_data;
					if(dev != scull_devices){ // only queue0 can have access to pop command 
						return -EACCES;
					}
					struct message *tmpMsg= (scull_devices+1+i)->head;
					(scull_devices+1+i)->head = (scull_devices+1+i)->head->next;
					if((scull_devices+1+i)->head)
						(scull_devices+1+i)->head->prev =NULL;
					tmpMsg->next = NULL;
					//tmpMsg = NULL;
					kfree(tmpMsg->text);
					kfree(tmpMsg);
					//retval = tmpMsg->text;
					printk("%s\n", tmpMsg->text);
					
					break;
				}
			}
			
			
		}
	  
	  
		break;

	  default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}
	return retval;
}


loff_t scull_llseek(struct file *filp, loff_t off, int whence)
{
    struct scull_dev *dev = filp->private_data;
    loff_t newpos;

    switch(whence) {
        case 0: /* SEEK_SET */
            newpos = off;
            break;

        case 1: /* SEEK_CUR */
            newpos = filp->f_pos + off;
            break;

        case 2: /* SEEK_END */
            newpos = dev->size + off;
            break;

        default: /* can't happen */
            return -EINVAL;
    }
    if (newpos < 0)
        return -EINVAL;
    filp->f_pos = newpos;
    return newpos;
}


struct file_operations scull_fops = {
    .owner =    THIS_MODULE,
    .llseek =   scull_llseek,
    .read =     scull_read,
    .write =    scull_write,
    .unlocked_ioctl =  scull_ioctl,
    .open =     scull_open,
    .release =  scull_release,
};


void scull_cleanup_module(void)
{
	
    int i;
    dev_t devno = MKDEV(scull_major, scull_minor);

    if (scull_devices) {
        for (i = 0; i < scull_nr_devs; i++) {
            scull_trim(scull_devices + i);
            struct scull_dev *dev = scull_devices+i;

    
			while(dev->head){
				struct message* temp = dev->head;
				char*tempmsg = dev->head->text;
				dev->head = dev->head->next;
				printk("Silinen: %s\n", tempmsg);
				temp->next= NULL;
				temp->prev = NULL;
				kfree(tempmsg);
				kfree(temp);
			}
            cdev_del(&scull_devices[i].cdev);
        }
    kfree(scull_devices);
    }
	
    unregister_chrdev_region(devno, scull_nr_devs);
}


int scull_init_module(void)
{
    int result, i;
    int err;
    dev_t devno = 0;
    struct scull_dev *dev;

    if (scull_major) {
        devno = MKDEV(scull_major, scull_minor);
        result = register_chrdev_region(devno, scull_nr_devs, "scull");
    } else {
        result = alloc_chrdev_region(&devno, scull_minor, scull_nr_devs,
                                     "scull");
        scull_major = MAJOR(devno);
    }
    if (result < 0) {
        printk(KERN_WARNING "scull: can't get major %d\n", scull_major);
        return result;
    }

    scull_devices = kmalloc(scull_nr_devs * sizeof(struct scull_dev),
                            GFP_KERNEL);
    if (!scull_devices) {
        result = -ENOMEM;
        goto fail;
    }
    memset(scull_devices, 0, scull_nr_devs * sizeof(struct scull_dev));

    /* Initialize each device. */
    for (i = 0; i < scull_nr_devs; i++) {
        dev = &scull_devices[i];
        dev->quantum = scull_quantum;
        dev->qset = scull_qset;
        sema_init(&dev->sem,1);
        devno = MKDEV(scull_major, scull_minor + i);
        cdev_init(&dev->cdev, &scull_fops);
        dev->cdev.owner = THIS_MODULE;
        dev->cdev.ops = &scull_fops;
        err = cdev_add(&dev->cdev, devno, 1);
        if (err)
            printk(KERN_NOTICE "Error %d adding scull%d", err, i);
    }

    return 0; /* succeed */

  fail:
    scull_cleanup_module();
    return result;
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);
