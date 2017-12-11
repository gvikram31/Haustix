#ifndef __KERNEL__
#define __KERNEL__
#endif

#ifndef MODULE
#define MODULE
#endif

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <asm/system.h> /* cli(), *_flags */
#include <asm/uaccess.h> /* copy_from/to_user */
#include <asm-arm/arch-pxa/gpio.h>
#include <linux/delay.h> /* MSLEEP */
#include <linux/interrupt.h>
#include <asm/arch/pxa-regs.h>
#include <asm-arm/arch/hardware.h>
#define PWM_PIR GPIO16_PWM0
#define CK_EN CKEN0_PWM0
void setDirs(void);
void setGpio(void);
static void count_handler(unsigned long);
static void pulsehigh_timer_handler(unsigned long data);
/* Major number */
static int mygpio_major = 61;
static int catflag = 0;
static struct timer_list * count_timer;
static struct timer_list * pulsehigh_timer;
static int pulse_dir = 990; //clockwise rotations
static int pulse_pin = 29;
/* Declaration of memory.c functions */
static int mygpio_open(struct inode *inode, struct file *filp);
static int mygpio_release(struct inode *inode, struct file *filp);
static ssize_t mygpio_read(struct file *filp,
		char *buf, size_t count, loff_t *f_pos);
static ssize_t mygpio_write(struct file *filp,
		const char *buf, size_t count, loff_t *f_pos);



struct file_operations mygpio_fops = {
	read: mygpio_read,
	write: mygpio_write,
	open: mygpio_open,
	release: mygpio_release
};


static void pulsehigh_timer_handler(unsigned long data) {
        gpio_set_value(pulse_pin,0);
        printk("pulse handler = %ld\r\n", jiffies);
}

static void count_handler(unsigned long data) {
		
        printk("counter handler time = %ld\r\n",  jiffies + usecs_to_jiffies(20));
        gpio_set_value(pulse_pin,1);
        mod_timer(pulsehigh_timer, jiffies + usecs_to_jiffies(pulse_dir));
        mod_timer(count_timer, jiffies + usecs_to_jiffies(20));
        
}


static int my_init_module(void)
{
	printk("Hello world!\n");
	int result;
	/* Registering device */
	result = register_chrdev(mygpio_major, "mygpio", &mygpio_fops);
	if (result < 0)
	{
		printk(KERN_ALERT
			"mygpio: cannot obtain major number %d\n", mygpio_major);
		return result;
	}
    //pulse direction output
    gpio_direction_output(pulse_pin,0);
    //setting two timers. count_timer will be 20 ms periodic and pulsehigh_timer will be 1 or 2ms periodic.
	//count_timer = (struct timer_list *) kmalloc(sizeof(struct timer_list), GFP_KERNEL);
    //pulsehigh_timer = (struct timer_list *) kmalloc(sizeof(struct timer_list), GFP_KERNEL);
    //setup_timer(count_timer, count_handler, 0);
    //setup_timer(pulsehigh_timer, pulsehigh_timer_handler, 0);
	//mod_timer(count_timer, jiffies + usecs_to_jiffies(20));
	return 0;
}

static void my_cleanup_module(void)
{
	unregister_chrdev(mygpio_major, "mygpio");
   // del_timer(count_timer);	
  //  del_timer(pulsehigh_timer);	
    gpio_set_value(pulse_pin,0);
	printk("Bye world!\n");
}

static ssize_t mygpio_write(struct file *filp, const char *buf,
							size_t count, loff_t *f_pos)
{
	char *name=(char *) kmalloc((size_t)1200,GFP_KERNEL);
	memset(name,0,sizeof(name));
//    printk("%s\n",buf);
    if(buf[0]=='1')
        {   
  //          pulse_dir = 1; //clockwise rotations
            gpio_set_value(pulse_pin,1);
            
        }
    else if(buf[0] == '0')
    {
//        pulse_dir = 2; //counterclockwise way
        gpio_set_value(pulse_pin,0);
    }
        
	return count;

}

static ssize_t mygpio_read(struct file *filp, char *buf, 
							size_t count, loff_t *f_pos)
{ 
	char* output_buffer=(char *) kmalloc((size_t)250,GFP_KERNEL);
	memset(output_buffer,0,(size_t)1024);

	int outcount=0;
	outcount=sprintf(output_buffer,"%d\n",pulse_dir);

	if(catflag==0)
	{
	int ret = copy_to_user(buf, output_buffer , outcount);
	if (ret)
	{
		return -EFAULT;
	}
	catflag=1;
	return outcount; 

	}
	else if(catflag==1)
	{
	return 0;
	}
}

static int mygpio_open(struct inode *inode, struct file *filp)
{

	/* Success */
	catflag=0;
	return 0;
}

static int mygpio_release(struct inode *inode, struct file *filp)
{

	/* Success */
	catflag=0;
	return 0;
}



module_init(my_init_module);
module_exit(my_cleanup_module);

