#ifndef __SCULL_H
#define __SCULL_H

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

#define SCULL_IOC_MAGIC  'k'
// maxnr 0 a cek, yeni bir iocqueuepop define et, IOR
#define SCULL_IOCQPOP     _IOR(SCULL_IOC_MAGIC, 0,int)

#define SCULL_IOC_MAXNR 0

#endif
