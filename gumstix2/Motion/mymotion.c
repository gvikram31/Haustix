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
void setDirs(void);
void setGpio(void);
void count_handler(unsigned long);
/* Major number */
static int mygpio_major = 61;
static unsigned PIR = 31;
//static int catflag = 0;
static unsigned led[2] = {29,30};
static struct timer_list * count_timer;
static int human_detect = 0;
/* Declaration of memory.c functions */
static int mygpio_open(struct inode *inode, struct file *filp);
static int mygpio_release(struct inode *inode, struct file *filp);
static ssize_t mygpio_read(struct file *filp,
		char *buf, size_t count, loff_t *f_pos);
//static ssize_t mygpio_write(struct file *filp,
//		const char *buf, size_t count, loff_t *f_pos);

struct file_operations mygpio_fops = {
	read: mygpio_read,
	//write: mygpio_write,
	open: mygpio_open,
	release: mygpio_release
};
void setGpio(void)
{
    //setting checking PIRs 
    //enabling the LED On an=d OFF
    if (human_detect == 1)
    {
        gpio_set_value(led[0],1);
        gpio_set_value(led[1],1);
    }
    else
    {
        gpio_set_value(led[0],0);
        gpio_set_value(led[1],0);
    }
}


void setDirs(void)
{
	//output led direction
    gpio_direction_output(led[0],0);
	gpio_direction_output(led[1],0);
}
irqreturn_t gpio_irq_pir(int irq, void *dev_id, struct pt_regs *regs)
{
	
    if(human_detect == 1){
        human_detect = 0;
        }
    else{
        human_detect = 1;
        }
   // setGpio();
	printk("Button IRQ PIR Sensor\n");
	return IRQ_HANDLED;
}

void count_handler(unsigned long data) {

		//mod_timer(count_timer, jiffies + msecs_to_jiffies(speeds[slevel]));	
		printk("Handler\n");
        pxa_gpio_mode(PIR | GPIO_IN);
	    int irq_pir = IRQ_GPIO(PIR);
	    if (request_irq(irq_pir, &gpio_irq_pir, SA_INTERRUPT | SA_TRIGGER_RISING,"mymotion", NULL) != 0 ) {
                printk ( "irq run not acquired \n" );
                return ;
        }else{
                printk ( "irq %d acquired successfully \n", irq_pir );
	}
        del_timer(count_timer);

}



static int my_init_module(void)
{
    int result;
	printk("Hello world!\n");	
	/* Registering device */
	result = register_chrdev(mygpio_major, "mymotion", &mygpio_fops);
	if (result < 0)
	{
		printk(KERN_ALERT
			"mymotion: cannot obtain major number %d\n", mygpio_major);
		return result;
	}
	count_timer = (struct timer_list *) kmalloc(sizeof(struct timer_list), GFP_KERNEL);
    setup_timer(count_timer, count_handler, 0);
	mod_timer(count_timer, jiffies + msecs_to_jiffies(5000));
    //setting PIR as an interrupt	
    
	//setDirs();
	//setGpio();
    
	return 0;
}

static void my_cleanup_module(void)
{
	
	free_irq(IRQ_GPIO(PIR), NULL);
	//if(count_timer)
	//del_timer(count_timer);
	unregister_chrdev(mygpio_major, "mygpio");
	printk("Bye world!\n");
}
/*
static ssize_t mygpio_write(struct file *filp, const char *buf,
							size_t count, loff_t *f_pos)
{
	char *name=(char *) kmalloc((size_t)1200,GFP_KERNEL);
	memset(name,0,sizeof(name));
	return count;

}
*/

static ssize_t mygpio_read(struct file *filp, char *buf, 
							size_t count, loff_t *f_pos)
{ 
	char* output_buffer=(char *) kmalloc((size_t)250,GFP_KERNEL);
    int outcount=0; int ret;
	memset(output_buffer,0,(size_t)1024);

	outcount=sprintf(output_buffer,"%d\n",human_detect);
	ret = copy_to_user(buf, output_buffer , outcount);
	if (ret)
	{
		return -EFAULT;
	}
	return outcount; 
}

static int mygpio_open(struct inode *inode, struct file *filp)
{

	/* Success */
	//catflag=0;
	return 0;
}

static int mygpio_release(struct inode *inode, struct file *filp)
{

	/* Success */
	//catflag=0;
	return 0;
}



module_init(my_init_module);
module_exit(my_cleanup_module);

