#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/ktime.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/sched.h>

struct my_node
{
	struct list_head list;
	int data;
};

struct list_head my_list_100000;

struct list_head* p, * pn;
struct my_node* current_node;
unsigned int n, random;

struct task_struct *thread1, *thread2, *thread3, *thread4;

int insert_data = 0;

spinlock_t lock;
spinlock_t mainlock;
// struct mutex my_mutex;

ktime_t start, end;

static int insert(void *data)
{
	struct my_node* new;

	while (true) {
		if (insert_data > 100000) break;
		spin_lock(&lock);
		// mutex_lock(&my_mutex);

		new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
		new->data = insert_data++;
		list_add_tail(&new->list, &my_list_100000);

		spin_unlock(&lock);
		// mutex_unlock(&my_mutex);
	}
	kthread_stop(pid_task(find_vpid((int) task_pid_nr(current)), PIDTYPE_PID));

	do_exit(0);
}

// static int search(void *data)
// {
// 	list_for_each_entry(current_node, &my_list_100000, list);
// 	end = ktime_get_ns();
// 	printk("total time(search): 0.%09lld secs\n", end - start);
// 	kthread_stop(pid_task(find_vpid((int) task_pid_nr(current)), PIDTYPE_PID));
// 	do_exit(0);
// }

// static int delete(void *data)
// {
// 	list_for_each_safe(p, pn, &my_list_100000) {
// 		current_node = list_entry(p, struct my_node, list);
// 		list_del(p);
// 	}	
// 	kfree(current_node);
// }

int __init llist_module_init(void)
{
	printk("llist module init\n");
	INIT_LIST_HEAD(&my_list_100000);
	// mutex_init(&my_mutex);

	spin_lock(&mainlock);
	start = ktime_get_ns();

	thread1 = kthread_run(insert, NULL, "insert");
	thread2 = kthread_run(insert, NULL, "insert");
	thread3 = kthread_run(insert, NULL, "insert");
	thread4 = kthread_run(insert, NULL, "insert");

	spin_lock(&mainlock);

	end = ktime_get_ns();
	printk("total time(insert): %lld ns\n", end - start);	

	// if (insert_data == 100000) {
	// 	end = ktime_get_ns();
	// 	printk("finish(%lld secs)\n", (end - start) / 1000000000);
	// }

	return 0;
}

void __exit llist_module_cleanup(void)
{
	// kthread_stop(thread1);
	// kthread_stop(thread2);
	// kthread_stop(thread3);
	// kthread_stop(thread4);
	printk("rm llist module\n\n");
}

module_init(llist_module_init);
module_exit(llist_module_cleanup);
MODULE_LICENSE("GPL");
