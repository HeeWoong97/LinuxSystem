#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

int counter;

struct task_struct *thread1, *thread2, *thread3, *thread4;

static int fetch_and_add(void *data)
{
    while (!kthread_should_stop()) {
        __sync_fetch_and_add(&counter, 1);
        printk("%s, counter: %d, thread pid: %u\n", __func__, counter, current->pid);
        msleep(500);
    }
    do_exit(0);
}

int __init mod_init(void)
{
    printk("%s, module init\n", __func__);
    thread1 = kthread_run(fetch_and_add, NULL, "fetch_and_add");
    thread2 = kthread_run(fetch_and_add, NULL, "fetch_and_add");
    thread3 = kthread_run(fetch_and_add, NULL, "fetch_and_add");
    thread4 = kthread_run(fetch_and_add, NULL, "fetch_and_add");
    return 0;
}

void __exit mod_exit(void)
{
    kthread_stop(thread1);
    kthread_stop(thread2);
    kthread_stop(thread3);
    kthread_stop(thread4);
    printk("%s, module exit\n", __func__);
}

module_init(mod_init);
module_exit(mod_exit);
MODULE_LICENSE("GPL");