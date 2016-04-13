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
    int val;
    int value;
    ret=-1;value=-1;
    printf("ret:%d,value:%d\n\n",ret,value);
    int fd=open("/dev/simple_char_drv",O_RDWR | O_NONBLOCK);
    if(fd<0)
    {
        printf("Can‘t open driver!\n");
        return -1;
    }
    
    //-------开-关LED灯--
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
    //------读取按键值状态--
    while (1)
	{
        //printf("sizeof(value):%d\n",sizeof(value));
       // printf("%d\n",sizeof(value));
		ret = read(fd, &value,1);
        printf("ret:%d,value:%d\n\n",ret,value);
	}
	
    return 0;
}
