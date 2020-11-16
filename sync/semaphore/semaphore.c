#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

int counter;
struct rw_semaphore counter_rwse;
struct task_struct* reader_thread1, * reader_thread2, * writer_thread1, * writer_thread2;

static int writer_function(void* data)
{
    while (!kthread_should_stop()) {
        down_write(&counter_rwse);
        counter++;
        printk("%s, writer counter: %d, pid: %u\n", __func__, counter, current->pid);
        up_write(&counter_rwse);
        msleep(500);
    }
    do_exit(0);
}

static int reader_function(void* data)
{
    while (!kthread_should_stop()) {
        down_read(&counter_rwse);
        printk("%s, reader counter: %d, pid: %u\n", __func__, counter, current->pid);
        up_read(&counter_rwse);
        msleep(500);
    }
    do_exit(0);
}

static int __init my_module_init(void)
{
    printk("%s, Entering module\n", __func__);
    counter = 0;
    init_rwsem(&counter_rwse);
    writer_thread1 = kthread_run(writer_function, NULL, "writer_function");
    writer_thread2 = kthread_run(writer_function, NULL, "writer_function");
    reader_thread1 = kthread_run(reader_function, NULL, "reader_function");
    reader_thread2 = kthread_run(reader_function, NULL, "reader_function");

    return 0;
}

static void __exit my_module_exit(void)
{
    kthread_stop(writer_thread1);
    kthread_stop(writer_thread2);
    kthread_stop(reader_thread1);
    kthread_stop(reader_thread2);

    printk("%s, Exiting module\n", __func__);
}

module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");