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

ktime_t start, end;

static int insert(void *data)
{
	struct my_node* new;

	while (!kthread_should_stop()) {
		if (insert_data > 100000) {
			break;
		}
		spin_lock(&lock);

		new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
		new->data = insert_data++;
		printk("pid(%u) insert %d\n", current->pid, insert_data);
		list_add_tail(&new->list, &my_list_100000);

		spin_unlock(&lock);
	}
	
	do_exit(0);
}

// static int search(void *data)
// {
// 	get_random_bytes(&n, sizeof(int)); // generate random number
// 	random = n % 100000;
// 	printk("random number: %d\n", random);
// 	list_for_each_entry(current_node, &my_list_100000, list) {
// 		if (current_node->data == random) {
// 			printk("find!\n");
// 		}
// 	}	
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

	start = ktime_get_ns();

	thread1 = kthread_run(insert, NULL, "insert");
	thread2 = kthread_run(insert, NULL, "insert");
	thread3 = kthread_run(insert, NULL, "insert");
	thread4 = kthread_run(insert, NULL, "insert");

	// if (insert_data > 100000) {
	// 	end = ktime_get_ns();

	// 	printk("finish(%lld secs)\n", (end - start) / 1000000000);

	// 	kthread_stop(thread1);
	// 	kthread_stop(thread2);
	// 	kthread_stop(thread3);
	// 	kthread_stop(thread4);
	// }

	return 0;
}

void __exit llist_module_cleanup(void)
{
	kthread_stop(thread1);
	kthread_stop(thread2);
	kthread_stop(thread3);
	kthread_stop(thread4);
	printk("rm llist module\n\n");
}

module_init(llist_module_init);
module_exit(llist_module_cleanup);
MODULE_LICENSE("GPL");