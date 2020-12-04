#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <asm/timex.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/mutex.h>

struct my_node
{
	int value;
	struct rb_node rb;
};

struct my_node_sync
{
	struct my_node node;
	// bool islocked;
	// pid_t pid;
	spinlock_t lock;
};

struct arguments
{
	struct my_node_sync *node;
	struct rb_root_cached *root;
};

struct mutex my_mutex;

// bool node_is_locked(struct my_node_sync *node)
// {
// 	return node->islocked;
// }

// bool unlock_node(struct my_node_sync *node)
// {
// 	if (current->pid == node->pid) {
// 		node->islocked = false;
// 		return true;
// 	}
// 	return false;
// }

// bool lock_node(struct my_node_sync *node)
// {
// 	if (node->islocked == false) {
// 		node->islocked = true;
// 		node->pid = current->pid;
// 		return true;
// 	}
// 	return false;
// }

struct task_struct *thread1, *thread2;

int count;

static struct rb_root_cached rbtree_100000_root = RB_ROOT_CACHED;
static struct rb_root_cached rbtree_sync_root = RB_ROOT_CACHED;

static struct my_node *rbtree_100000 = NULL;
static struct my_node_sync *rbtree_sync = NULL;

static struct rnd_state rnd;

void insert(struct my_node *node, struct rb_root_cached *root)
{
	struct rb_node **new = &root->rb_root.rb_node, *parent = NULL;
	int value = node->value;

	while (*new) {
		parent = *new;
		if (value < rb_entry(parent, struct my_node, rb)-> value) {
			new = &parent->rb_left;
		}
		else {
			new = &parent->rb_right;
		}
	}

	rb_link_node(&node->rb, parent, new);
	rb_insert_color(&node->rb, &root->rb_root);
}

static inline void erase(struct my_node *node, struct rb_root_cached *root)
{
	rb_erase(&node->rb, &root->rb_root);
}

static int erase_sync(void *arguments)
{
	// struct arguments *args = arguments;
	// struct my_node_sync *targetnode = args->node;
	// struct rb_root_cached *targetroot = args->root;

	while (1) {
		if (count == 50) {
			break;
		}
		// if (targetnode->islocked) { // if node is locked
		// 	continue;
		// }
		// lock_node(targetnode);
		mutex_lock(&my_mutex);
		// printk("pid %d deleted %d node(value: %d)\n", current->pid, targetnode->node.value, count);
		// rb_erase(&(targetnode->node.rb), &(targetroot->rb_root));
		printk("pid: %d, count: %d\n", current->pid, count++);
		// unlock_node(targetnode);
		mutex_unlock(&my_mutex);
		// count++;
		// msleep(500);
		// targetnode += count;
		msleep(1);
	}
	kthread_stop(pid_task(find_vpid((int) task_pid_nr(current)), PIDTYPE_PID));
	do_exit(0);
}

static void init(void)
{
	int i;
	for (i = 0; i < 100000; i++) {
		rbtree_100000[i].value = prandom_u32_state(&rnd);

		rbtree_sync[i].node.value = prandom_u32_state(&rnd);
		// rbtree_sync[i].islocked = false;
		// rbtree_sync[i].pid = 0;
		spin_lock_init(&rbtree_sync[i].lock);
	}	
}

void RB_example(void)
{	
	int i;
	ktime_t start, end;
	struct rb_node *node;
	struct arguments args;

	rbtree_100000 = kmalloc_array(100000, sizeof(*rbtree_100000), GFP_KERNEL);
	rbtree_sync = kmalloc_array(100000, sizeof(*rbtree_sync), GFP_KERNEL);

	prandom_seed_state(&rnd, 3141592653589793238ULL);
	init();

	////////// insert //////////
	// printk("\n////////// insert //////////\n");

	/* insert 100000 entries */
	start = ktime_get();

	for (i = 0; i < 100000; i++) {
		insert(rbtree_100000 + i, &rbtree_100000_root);
	}
	
	end = ktime_get();

	printk("insert(normal version): 0.%09lld secs\n", end - start);

	start = ktime_get();

	for (i = 0; i < 100000; i++) {
		insert(&(rbtree_sync[i].node), &rbtree_sync_root);
	}
	
	end = ktime_get();

	printk("insert(sync version): 0.%09lld secs\n", end - start);

	////////// traverse //////////
	// printk("\n////////// search //////////\n");

	/* traverse 100000 entries */
	// start = ktime_get();

	// for (node = rb_first(&rbtree_100000_root.rb_root); node; node = rb_next(node));

	// end = ktime_get();

	// printk("search(100000 entries): 0.%09lld secs\n", end - start);

	////////// delete //////////
	// printk("\n////////// delete //////////\n");

	/* delete 100000 entries */
	start = ktime_get();

	for (i = 0; i < 100000; i++) {
		erase(rbtree_100000 + i, &rbtree_100000_root);
	}

	end = ktime_get();

	args.node = rbtree_sync;
	args.root = &rbtree_sync_root;

	// thread1 = kthread_run(erase_sync, &args, "erase_sync");
	// thread2 = kthread_run(erase_sync, &args, "erase_sync");
	thread1 = kthread_run(erase_sync, NULL, "erase_sync");
	thread2 = kthread_run(erase_sync, NULL, "erase_sync");

	// printk("delete(100000 entries): 0.%09lld secs\n", end - start);
}

int __init rbtree_module_init(void)
{
	struct task_struct *result = pid_task(find_vpid((int) task_pid_nr(current)), PIDTYPE_PID);
	ktime_t start, end;
	int i;

	count = 0;
	
	mutex_init(&my_mutex);

	start = ktime_get();

	printk("\n********** rbtree testing!! **********\n");
	printk("scheduling policy: %d\n", result->policy);
	printk("first vruntime: %lld\n", result->se.vruntime);

	RB_example();

	printk("second vruntime: %lld\n\n", result->se.vruntime);

	end = ktime_get();
	printk("total time: 0.%09lld secs\n", end - start);

	return 0;
}

void __exit rbtree_module_cleanup(void)
{
	// kthread_stop(thread1);
	// kthread_stop(thread2);
	printk("\nBye module\n");
}

module_init(rbtree_module_init);
module_exit(rbtree_module_cleanup);
MODULE_LICENSE("GPL");
