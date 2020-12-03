#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

volatile int lock = 0;

struct task_struct *thread1, *thread2, *thread3, *thread4;

static int test_and_set(void *data)
{
    while (!kthread_should_stop()) {
        while (__sync_lock_test_and_set(&lock, 1)) {
            while (lock);
        }
        printk("thread pid: %u gets the lock!\n", current->pid);
        __sync_lock_release(&lock);
        msleep(500);
    }
    do_exit(0);
}

int __init mod_init(void)
{
    printk("module init\n");
    thread1 = kthread_run(test_and_set, NULL, "test_and_set");
    thread2 = kthread_run(test_and_set, NULL, "test_and_set");
    thread3 = kthread_run(test_and_set, NULL, "test_and_set");
    thread4 = kthread_run(test_and_set, NULL, "test_and_set");
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