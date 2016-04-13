/*************************************************************************
    > File Name: driver_test.c
    > Author: 浅握双手
    > ---------------- 
    > Created Time: 2016年04月13日 11时09分29秒 CST
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc,char **argv)
{
    int ret;
    unsigned char value;
    int val;
    
    int fd=open("/dev/simple_char_drv",O_RDWR | O_NONBLOCK);
    if(fd<0)
    {
        printf("Cant open driver!\n");
        return -1;
    }
    
    ret = read(fd, &value, 1);
    printf("ret:%d,value:%d\n\n",ret,value);
    
    for(val=4;val<=6;val++)
    {
        write(fd, &val, 4);
        sleep(1);
    }
    
    for(val=-6;val<=-4;val++)
    {
        write(fd, &val, 4);
        sleep(1);
    }
    return 0;
}
