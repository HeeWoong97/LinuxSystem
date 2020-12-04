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

struct task_struct *thread1, *thread2, *thread3, *thread4;

struct my_node
{
	int value;
	struct rb_node rb;
};

static struct rb_root_cached rbtree1_root = RB_ROOT_CACHED;
static struct rb_root_cached rbtree2_root = RB_ROOT_CACHED;
static struct rb_root_cached rbtree3_root = RB_ROOT_CACHED;
static struct rb_root_cached rbtree4_root = RB_ROOT_CACHED;

static struct my_node *rbtree1 = NULL;
static struct my_node *rbtree2 = NULL;
static struct my_node *rbtree3 = NULL;
static struct my_node *rbtree4 = NULL;

static struct rnd_state rnd;

spinlock_t lock;

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
	for (i = 0; i < 25000; i++) {
		insert(rbtree1 + i, &rbtree1_root);
	}	
	do_exit(0);
}

static int insert2(void *data)
{
	int i;
	for (i = 0; i < 25000; i++) {
		insert(rbtree2 + i, &rbtree2_root);
	}
	do_exit(0);	
}

static int insert3(void *data)
{
	int i;
	for (i = 0; i < 25000; i++) {
		insert(rbtree3 + i, &rbtree3_root);
	}
	do_exit(0);	
}

static int insert4(void *data)
{
	int i;
	for (i = 0; i < 25000; i++) {
		insert(rbtree4 + i, &rbtree4_root);
	}
	do_exit(0);	
}

static inline void erase(struct my_node *node, struct rb_root_cached *root)
{
	rb_erase(&node->rb, &root->rb_root);
}

static void init(void)
{
	int i;
	for (i = 0; i < 25000; i++) {
		rbtree1[i].value = prandom_u32_state(&rnd);
		rbtree2[i].value = prandom_u32_state(&rnd);
		rbtree3[i].value = prandom_u32_state(&rnd);
		rbtree4[i].value = prandom_u32_state(&rnd);
	}	
}

void RB_example(void)
{	
	int i;
	ktime_t start, end;
	struct rb_node *node;

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

	spin_lock(&lock);
	start = ktime_get_ns();

	thread1 = kthread_run(insert1, NULL, "insert1");
	thread2 = kthread_run(insert2, NULL, "insert2");
	thread3 = kthread_run(insert3, NULL, "insert3");
	thread4 = kthread_run(insert4, NULL, "insert4");

	spin_unlock(&lock);

	end = ktime_get_ns();
	printk("insert: %lld ns\n", end - start);

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
	// start = ktime_get();

	// for (i = 0; i < 100000; i++) {
	// 	erase(rbtree_100000 + i, &rbtree_100000_root);
	// }

	// end = ktime_get();
	// printk("delete: %lld ns\n", end - start);
}

int __init rbtree_module_init(void)
{
	RB_example();

	return 0;
}

void __exit rbtree_module_cleanup(void)
{
	kthread_stop(thread1);
	kthread_stop(thread2);
	kthread_stop(thread3);
	kthread_stop(thread4);

	printk("\nBye module\n");
}

module_init(rbtree_module_init);
module_exit(rbtree_module_cleanup);
MODULE_LICENSE("GPL");
