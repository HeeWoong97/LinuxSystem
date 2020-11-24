#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>  // for threads
#include <linux/sched.h>  // for task_struct
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>


/*char buffer[10];
int count;
struct semaphore sem;*/
static struct task_struct *thread1;



int thread_fn(void) {

unsigned long j0,j1;
int delay = 100*HZ;
j0 = jiffies;
j1 = j0 + delay;

while (time_before(jiffies, j1))
        schedule();
printk(KERN_INFO "In thread1");


set_current_state(TASK_INTERRUPTIBLE);
while (!kthread_should_stop())
{
   schedule();
   set_current_state(TASK_INTERRUPTIBLE);
}
set_current_state(TASK_RUNNING);

return 0;
}


int suspendstate(char *buf,char **start,off_t offset,int count,int *eof,void *data)
{
    printk(KERN_INFO "In suspend sate");
    thread1->state = TASK_INTERRUPTIBLE;
    return 0;
}

int runstate(char *buf,char **start,off_t offset,int count,int *eof,void *data)
{
    printk(KERN_INFO "In runsate");
     wake_up_process(thread1);   
    return 0;
}

int thread_init (void) {
   
    char name[15]="process1";
    printk(KERN_INFO "in init");
    thread1 = kthread_create(thread_fn,NULL,name);
    if((thread1))
        {
        printk(KERN_INFO "in if");
        wake_up_process(thread1);
        }
    create_proc_read_entry("suspend_state",0,NULL,suspendstate,NULL);
    create_proc_read_entry("run_state",0,NULL,runstate,NULL);
    return 0;
}



void thread_cleanup(void) {
    int ret;
    ret = kthread_stop(thread1);
    if(!ret)
        printk(KERN_INFO "Thread stopped");

}
MODULE_LICENSE("GPL");   
module_init(thread_init);
module_exit(thread_cleanup);
