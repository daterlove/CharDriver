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

static struct class *char_drv_class;
static struct class_device	*char_drv_class_dev;
int major;//保存主设备号

volatile unsigned int *gpfcon = NULL;
volatile unsigned int *gpfdat = NULL;

static int char_drv_open(struct inode *inode, struct file *file)
{
    printk("driver open...\n");

	return 0;
}

static ssize_t char_drv_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
    printk("driver write...\n");
	return 0;
}

ssize_t char_drv_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	printk("driver read...\n");
	return 0;
}

static struct file_operations char_drv_fops = {
    .owner  =   THIS_MODULE,    // 宏，指向编译模块时自动创建的__this_module变量 
    .open   =   char_drv_open,     
	.write	=	char_drv_write,
    .read	 =	char_drv_read,		   
};

static int char_drv_init(void)
{
    printk("driver init...\n");
	major = register_chrdev(0, "char_drv", &char_drv_fops); //在内核注册，由内核分配主设备号

	char_drv_class = class_create(THIS_MODULE, "char_drv");
	char_drv_class_dev = class_device_create(char_drv_class, NULL, MKDEV(major, 0), NULL, "simple_char_drv"); /* 创建设备节点*/

    //映射物理地址
    gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;
    
    *gpfcon &= ~((0x3<<(4*2)) | (0x3<<(5*2)) | (0x3<<(6*2)));//设零
	*gpfcon |= ((0x1<<(4*2)) | (0x1<<(5*2)) | (0x1<<(6*2)));//设一

    *gpfdat &= ~((1<<4) | (1<<5) | (1<<6));
	return 0;
}

static void char_drv_exit(void)
{
    printk("driver exit...\n");
	unregister_chrdev(major, "char_drv"); // 卸载

	class_device_unregister(char_drv_class_dev);
	class_destroy(char_drv_class);
    
    iounmap(gpfcon);//接触MMU映射
}

module_init(char_drv_init);
module_exit(char_drv_exit);


MODULE_LICENSE("GPL");

