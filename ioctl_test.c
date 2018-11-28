#include <stdio.h>
#include <fcntl.h>
#include "scull_ioctl.h"  //ioctl header file

main ( ) {
		int fd;
        fd = open("/dev/scull0", O_RDWR);

        if (fd == -1)
        {
                printf("Error in opening file \n");
                return -1;
        }
       
		ioctl(fd,SCULL_IOCQPOP);  //ioctl call 

         close(fd);
}
