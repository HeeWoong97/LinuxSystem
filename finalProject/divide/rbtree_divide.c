#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <asm/timex.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include <linux/delay.h>

struct task_struct *thread1, *thread2, *thread3, *thread4;

struct my_node
{
	int value;
	struct rb_node rb;
};

static struct rb_root_cached rbtree_root = RB_ROOT_CACHED;

static struct rb_root_cached rbtree1_root = RB_ROOT_CACHED;
static struct rb_root_cached rbtree2_root = RB_ROOT_CACHED;
static struct rb_root_cached rbtree3_root = RB_ROOT_CACHED;
static struct rb_root_cached rbtree4_root = RB_ROOT_CACHED;

static struct my_node *rbtree = NULL;

static struct my_node *rbtree1 = NULL;
static struct my_node *rbtree2 = NULL;
static struct my_node *rbtree3 = NULL;
static struct my_node *rbtree4 = NULL;

static struct rnd_state rnd;

spinlock_t lock;

int running_thread = 0;

int check = 0;

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

static int insert1(void *data)
{
	int i;
	running_thread++;
	for (i = 0; i < 25000; i++) {
		insert(rbtree1 + i, &rbtree1_root);
	}	
	running_thread--;
	kthread_stop(thread1);
	return 0;
}

static int insert2(void *data)
{
	int i;
	running_thread++;
	for (i = 0; i < 25000; i++) {
		insert(rbtree2 + i, &rbtree2_root);
	}
	running_thread--;
	kthread_stop(thread2);
	return 0;
}

static int insert3(void *data)
{
	int i;
	running_thread++;
	for (i = 0; i < 25000; i++) {
		insert(rbtree3 + i, &rbtree3_root);
	}
	running_thread--;
	kthread_stop(thread3);
	return 0;
}

static int insert4(void *data)
{
	int i;
	running_thread++;
	for (i = 0; i < 25000; i++) {
		insert(rbtree4 + i, &rbtree4_root);
	}
	running_thread--;
	kthread_stop(thread4);
	return 0;	
}

static inline void erase(struct my_node *node, struct rb_root_cached *root)
{
	rb_erase(&node->rb, &root->rb_root);
}

static int erase1(void *data)
{
	int i;
	running_thread++;
	for (i = 0; i < 100000; i++) {
		erase(rbtree1 + i, &rbtree1_root);
	}
	running_thread--;
	kthread_stop(thread1);
	return 0;
}

static int erase2(void *data)
{
	int i;
	running_thread++;
	for (i = 0; i < 100000; i++) {
		erase(rbtree2 + i, &rbtree2_root);
	}
	running_thread--;
	kthread_stop(thread2);
	return 0;
}

static int erase3(void *data)
{
	int i;
	running_thread++;
	for (i = 0; i < 100000; i++) {
		erase(rbtree3 + i, &rbtree3_root);
	}
	running_thread--;
	kthread_stop(thread3);
	return 0;
}

static int erase4(void *data)
{
	int i;
	running_thread++;
	for (i = 0; i < 100000; i++) {
		erase(rbtree4 + i, &rbtree4_root);
	}
	running_thread--;
	kthread_stop(thread4);
	return 0;
}

static void init(void)
{
	int i;
	for (i = 0; i < 100000; i++) {
		if (i < 25000) {
			rbtree[i].value = prandom_u32_state(&rnd);
			rbtree1[i].value = prandom_u32_state(&rnd);
			rbtree2[i].value = prandom_u32_state(&rnd);
			rbtree3[i].value = prandom_u32_state(&rnd);
			rbtree4[i].value = prandom_u32_state(&rnd);
			continue;
		}
		rbtree[i].value = prandom_u32_state(&rnd);
	}	
}

int __init rbtree_module_init(void)
{
	int i;
	ktime_t start, end;
	struct rb_node *node;

	rbtree = kmalloc_array(100000, sizeof(*rbtree), GFP_KERNEL);
	rbtree1 = kmalloc_array(25000, sizeof(*rbtree1), GFP_KERNEL);
	rbtree2 = kmalloc_array(25000, sizeof(*rbtree2), GFP_KERNEL);
	rbtree3 = kmalloc_array(25000, sizeof(*rbtree3), GFP_KERNEL);
	rbtree4 = kmalloc_array(25000, sizeof(*rbtree4), GFP_KERNEL);

	prandom_seed_state(&rnd, 3141592653589793238ULL);
	init();

	spin_lock_init(&lock);

	////////// insert //////////
	// printk("\n////////// insert //////////\n");

	/* insert 100000 entries */

	start = ktime_get_ns();

	for (i = 0; i < 100000; i++) {
		insert(rbtree + i, &rbtree_root);
	}

	end = ktime_get_ns();
	printk("insert(normal): %lld ns\n", end - start);

	// spin_lock(&lock);
	start = ktime_get_ns();

	thread1 = kthread_run(insert1, NULL, "insert1");
	thread2 = kthread_run(insert2, NULL, "insert2");
	thread3 = kthread_run(insert3, NULL, "insert3");
	thread4 = kthread_run(insert4, NULL, "insert4");

	while (running_thread > 0) {
		msleep(1);
	}
	end = ktime_get_ns();
	printk("insert(threaded): %lld ns\n", end - start);

	thread1 = NULL;
	thread2 = NULL;
	thread3 = NULL;
	thread4 = NULL;

	// spin_unlock(&lock);
	// end = ktime_get_ns();
	// printk("insert(threaded): %lld ns\n", end - start);

	////////// traverse //////////
	// printk("\n////////// search //////////\n");

	/* traverse 100000 entries */
	// start = ktime_get_ns();

	// for (node = rb_first(&rbtree_100000_root.rb_root); node; node = rb_next(node));

	// end = ktime_get_ns();
	// printk("traverse: %lld ns\n", end - start);

	////////// delete //////////
	// printk("\n////////// delete //////////\n");

	/* delete 100000 entries */
	start = ktime_get();

	for (i = 0; i < 100000; i++) {
		erase(rbtree + i, &rbtree_root);
	}

	end = ktime_get();
	printk("delete(normal): %lld ns\n", end - start);

	start = ktime_get_ns();

	thread1 = kthread_run(erase1, NULL, "erase1");
	thread2 = kthread_run(erase2, NULL, "erase2");
	thread3 = kthread_run(erase3, NULL, "erase3");
	thread4 = kthread_run(erase4, NULL, "erase4");

	while (running_thread > 0) {
		msleep(1);
	}
	end = ktime_get_ns();
	printk("delete(threaded): %lld ns\n", end - start);

	kfree(rbtree);
	kfree(rbtree1);
	kfree(rbtree2);
	kfree(rbtree3);
	kfree(rbtree4);

	return 0;
}

void __exit rbtree_module_cleanup(void)
{
	// kthread_stop(thread1);
	// kthread_stop(thread2);
	// kthread_stop(thread3);
	// kthread_stop(thread4);

	printk("\nBye module\n");
}

module_init(rbtree_module_init);
module_exit(rbtree_module_cleanup);
MODULE_LICENSE("GPL");
