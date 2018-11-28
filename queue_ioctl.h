#ifndef __QUEUE_H
#define __QUEUE_H

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

#define QUEUE_IOC_MAGIC  'k'
// maxnr 0 a cek, yeni bir iocqueuepop define et, IOR
#define QUEUE_IOCQPOP     _IOR(QUEUE_IOC_MAGIC, 0,char*)

#define QUEUE_IOC_MAXNR 0

#endif
