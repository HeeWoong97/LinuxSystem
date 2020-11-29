#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h> //task_pid_nr

/* This function is called when the module is loaded. */
int simple_init(void)
{
    u64 vruntime;
    struct task_struct *result;
    
    result = pid_task(find_vpid((int) task_pid_nr(current)), PIDTYPE_PID);
    vruntime = result->se.vruntime;

    printk("vruntime: %lld", vruntime);
    printk("vruntime: %lld", result->se.vruntime);

    printk("\n\n");
   //return -1; //debug mode of working
   return 0;
}

/* This function is called when the module is removed. */
void simple_exit(void) {
    printk(KERN_INFO "Removing Module\n");
}

/* Macros for registering module entry and exit points. */
module_init( simple_init );
module_exit( simple_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Module");
MODULE_AUTHOR("SGG");