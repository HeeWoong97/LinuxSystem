#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

int value = 0;

struct task_struct *thread1, *thread2, *thread3, *thread4;

static int compare_and_swap(void *data)
{
    while (!kthread_should_stop()) {
        __sync_val_compare_and_swap(&value, value, value + 1); 
        printk("thread(pid: %u) changes value from %d to %d\n", current->pid, value - 1, value);
        msleep(500);
    }
    do_exit(0);
}

int __init mod_init(void)
{
    printk("module init\n");
    thread1 = kthread_run(compare_and_swap, NULL, "compare_and_swap");
    thread2 = kthread_run(compare_and_swap, NULL, "compare_and_swap");
    thread3 = kthread_run(compare_and_swap, NULL, "compare_and_swap");
    thread4 = kthread_run(compare_and_swap, NULL, "compare_and_swap");
    return 0;
}

void __exit mod_exit(void)
{
    kthread_stop(thread1);
    kthread_stop(thread2);
    kthread_stop(thread3);
    kthread_stop(thread4);
    printk("module exit\n");
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");