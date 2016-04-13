#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/irq.h>

static struct class *char_drv_class;
static struct class_device	*char_drv_class_dev;
int major;//保存主设备号

volatile unsigned int *gpfcon = NULL;
volatile unsigned int *gpfdat = NULL;

static DECLARE_WAIT_QUEUE_HEAD(btn_waitq);//生成一个等待队列头


static volatile int ev_press = 0;// 中断事件标志, 中断服务程序将它置1，read将它清0
static int key_val;//按键值，按下1,2,3，松开-1，-2，-3

struct pin_desc
{
	unsigned int pin;
    int key_val;
};
struct  pin_desc pin_des[3]=
{
    {S3C2410_GPF0, 0x01},
    {S3C2410_GPF2, 0x02},
    {S3C2410_GPG3, 0x03}
};

static int char_drv_open(struct inode *inode, struct file *file)
{
    printk("driver open...\n");
    //配置LED为输出引脚
    *gpfcon &= ~((0x3<<(4*2)) | (0x3<<(5*2)) | (0x3<<(6*2)));//设零
	*gpfcon |= ((0x1<<(4*2)) | (0x1<<(5*2)) | (0x1<<(6*2)));//设一
	return 0;
}

static ssize_t char_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
    int val,open_flag;
    printk("driver write...\n");
	copy_from_user(&val, buf, count); //拷贝数据
    //printk("val:%d\n",val);

    if(val<0)
    {
        open_flag=0;
        val=-val;//转为正数
    }
    else
    {
       open_flag=1; 
    }
    
    if(val<4 || val>6)
    {
        return -1;        
    }
	if (open_flag)
	{
		*gpfdat &= ~((0x1<<val));//点亮LED
	}
	else
	{
		*gpfdat |= (0x1<<val);//灭掉LED
	}
	
	return 0;
}

ssize_t char_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	//printk("driver read...\n");
    if(size != 4)
        return -1;
        
    wait_event_interruptible(btn_waitq, ev_press);//阻塞进程
    ev_press = 0;
    
    //返回键值
    copy_to_user(buf, &key_val, 4);
	return 4;
}

static struct file_operations char_drv_fops = 
{
    .owner  =   THIS_MODULE,    // 宏，指向编译模块时自动创建的__this_module变量 
    .open   =   char_drv_open,     
	.write	=	char_drv_write,
    .read	 =	char_drv_read,		   
};

static irqreturn_t btn_irq(int irq, void *dev_id)
{
    struct pin_desc *pin_temp =(struct pin_desc*)dev_id;
    unsigned int pin_val;
    pin_val=s3c2410_gpio_getpin(pin_temp->pin);
    if(pin_val)//松开
    {
        key_val =-(pin_temp->key_val);
    }
    else
    {
        key_val = pin_temp->key_val;
    }
    
    ev_press = 1;//表示中断发生
    wake_up_interruptible(&btn_waitq); //唤醒休眠的进程
    
    return IRQ_RETVAL(IRQ_HANDLED);
}
static int char_drv_init(void)
{
    printk("driver init...\n");
	major = register_chrdev(0, "char_drv", &char_drv_fops); //在内核注册，由内核分配主设备号

	char_drv_class = class_create(THIS_MODULE, "char_drv");
	char_drv_class_dev = class_device_create(char_drv_class, NULL, MKDEV(major, 0), NULL, "simple_char_drv"); /* 创建设备节点*/

    //映射物理地址
    gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	// 配置GPF0，GPF2，GPG3为输入引脚 */
    //参数：（申请的硬件中断号，中断处理函数，中断处理属性，中断名称，传入中断处理程序的参数）
	request_irq(IRQ_EINT0,  btn_irq, IRQT_BOTHEDGE, "btn1", &pin_des[0]);//注册中断服务
	request_irq(IRQ_EINT2,  btn_irq, IRQT_BOTHEDGE, "btn2", &pin_des[1]);
	request_irq(IRQ_EINT11, btn_irq, IRQT_BOTHEDGE, "btn3", &pin_des[2]);
	return 0;
}

static void char_drv_exit(void)
{
    printk("driver exit...\n");
	unregister_chrdev(major, "char_drv"); // 卸载

	class_device_unregister(char_drv_class_dev);
	class_destroy(char_drv_class);
    
    iounmap(gpfcon);//解除MMU映射
    
    free_irq(IRQ_EINT0 , &pin_des[0]);//释放分配给已定中断
	free_irq(IRQ_EINT2 , &pin_des[1]);
	free_irq(IRQ_EINT11, &pin_des[2]);
}

module_init(char_drv_init);
module_exit(char_drv_exit);


MODULE_LICENSE("GPL");

