#include <stdio.h>
#include <fcntl.h>

#include "queue_ioctl.h"  //ioctl header file

main ( ) {
		int fd;
        fd = open("/dev/queue1", O_RDONLY);

        if (fd == -1)
        {
                printf("Error in opening file \n");
                return -1;
        }
        char * buf= malloc(10* sizeof(char*));
        
		ioctl(fd,QUEUE_IOCQPOP,buf);  //ioctl call 
		printf("%s\n",buf);
		free(buf);
        close(fd);
}
