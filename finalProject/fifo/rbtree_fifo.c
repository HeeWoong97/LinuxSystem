#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <asm/timex.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include <linux/types.h>

struct task_struct *result;

struct my_node
{
	int value;
	struct rb_node rb;
};

struct sched_param {
	int sched_priority;
};

struct sched_attr {
	__u32 size;

	__u32 sched_policy;
	__u64 sched_flags;

	/* SCHED_NORMAL, SCHED_BATCH */
	__s32 sched_nice;

	/* SCHED_FIFO, SCHED_RR */
	__u32 sched_priority;

	/* SCHED_DEADLINE */
	__u64 sched_runtime;
	__u64 sched_deadline;
	__u64 sched_period;

	/* Utilization hints */
	__u32 sched_util_min;
	__u32 sched_util_max;

};

static struct rb_root_cached rbtree_1000_root = RB_ROOT_CACHED;
static struct rb_root_cached rbtree_10000_root = RB_ROOT_CACHED;
static struct rb_root_cached rbtree_100000_root = RB_ROOT_CACHED;

static struct my_node *rbtree_1000 = NULL;
static struct my_node *rbtree_10000 = NULL;
static struct my_node *rbtree_100000 = NULL;

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

static void init(void)
{
	int i;
	for (i = 0; i < 100000; i++) {
		if (i < 1000) {
			rbtree_1000[i].value = prandom_u32_state(&rnd);
			rbtree_10000[i].value = prandom_u32_state(&rnd);
			rbtree_100000[i].value = prandom_u32_state(&rnd);
			continue;
		}
		else if (i < 10000) {
			rbtree_10000[i].value = prandom_u32_state(&rnd);
			rbtree_100000[i].value = prandom_u32_state(&rnd);
			continue;
		}
		else {
			rbtree_100000[i].value = prandom_u32_state(&rnd);
		}
	}	
}

void RB_example(void)
{	
	int i;
	ktime_t start, end;
	struct rb_node *node;

	rbtree_1000 = kmalloc_array(1000, sizeof(*rbtree_1000), GFP_KERNEL);
	rbtree_10000 = kmalloc_array(10000, sizeof(*rbtree_10000), GFP_KERNEL);
	rbtree_100000 = kmalloc_array(100000, sizeof(*rbtree_100000), GFP_KERNEL);

	prandom_seed_state(&rnd, 3141592653589793238ULL);
	init();

	////////// insert //////////
	// printk("\n////////// insert //////////\n");

	/* insert 1000 entries */
	start = ktime_get();

	for (i = 0; i < 1000; i++) {
		insert(rbtree_1000 + i, &rbtree_1000_root);
	}
	
	end = ktime_get();

	// printk("insert(1000 entries): 0.%09lld secs\n", end - start);

	/* insert 10000 entries */
	start = ktime_get();

	for (i = 0; i < 10000; i++) {
		insert(rbtree_10000 + i, &rbtree_10000_root);
	}
	
	end = ktime_get();

	// printk("insert(10000 entries): 0.%09lld secs\n", end - start);

	/* insert 100000 entries */
	start = ktime_get();

	for (i = 0; i < 100000; i++) {
		insert(rbtree_100000 + i, &rbtree_100000_root);
	}
	
	end = ktime_get();

	// printk("insert(100000 entries): 0.%09lld secs\n", end - start);
	// printk("rt_priority(after insert): %d\n", result->rt_priority);

	////////// traverse //////////
	// printk("\n////////// search //////////\n");

	/* traverse 1000 entries */
	start = ktime_get();

	for (node = rb_first(&rbtree_1000_root.rb_root); node; node = rb_next(node));

	end = ktime_get();

	// printk("search(1000 entries): 0.%09lld secs\n", end - start);

	/* traverse 10000 entries */
	start = ktime_get();

	for (node = rb_first(&rbtree_10000_root.rb_root); node; node = rb_next(node));

	end = ktime_get();

	// printk("search(10000 entries): 0.%09lld secs\n", end - start);

	/* traverse 100000 entries */
	start = ktime_get();

	for (node = rb_first(&rbtree_100000_root.rb_root); node; node = rb_next(node));

	end = ktime_get();

	// printk("search(100000 entries): 0.%09lld secs\n", end - start);
	// printk("rt_priority(after search): %d\n", result->rt_priority);

	////////// delete //////////
	// printk("\n////////// delete //////////\n");

	/* delete 1000 entries */
	start = ktime_get();

	for (i = 0; i < 1000; i++) {
		erase(rbtree_1000 + i, &rbtree_1000_root);
	}

	end = ktime_get();

	// printk("delete(1000 entries): 0.%09lld secs\n", end - start);

	/* delete 10000 entries */
	start = ktime_get();

	for (i = 0; i < 10000; i++) {
		erase(rbtree_10000 + i, &rbtree_10000_root);
	}

	end = ktime_get();

	// printk("delete(10000 entries): 0.%09lld secs\n", end - start);

	/* delete 100000 entries */
	start = ktime_get();

	for (i = 0; i < 100000; i++) {
		erase(rbtree_100000 + i, &rbtree_100000_root);
	}

	end = ktime_get();

	// printk("delete(100000 entries): 0.%09lld secs\n", end - start);
	// printk("rt_priority(after delete): %d\n", result->rt_priority);
}

int __init rbtree_module_init(void)
{
	struct sched_attr attr;
	// struct sched_param param;
	ktime_t start, end;
	int i;

	start = ktime_get();

	result = pid_task(find_vpid((int) task_pid_nr(current)), PIDTYPE_PID);

	// param.sched_priority = 99;
    memset(&attr, 0, sizeof(attr));
    attr.size = sizeof(struct sched_attr);
	attr.sched_policy = SCHED_FIFO;
	attr.sched_priority = 99;

	sched_setattr(result, &attr);
	// sched_setscheduler(result, SCHED_FIFO, &param);

	printk("\n********** rbtree_fifo testing!! **********\n");

	printk("rt_priority: %d\n", result->rt_priority);
	printk("scheduling policy: %d\n", result->policy);
	printk("first vruntime: %lld\n", result->se.vruntime);

	for (i = 0; i < 10; i++) {
		// printk("\n////////// loop %d //////////\n", i + 1);
		RB_example();
		// printk("\n");
	}

	printk("second vruntime: %lld\n\n", result->se.vruntime);

	end = ktime_get();
	printk("total time(fifo): 0.%09lld secs\n", end - start);

	return 0;
}

void __exit rbtree_module_cleanup(void)
{
	printk("\nBye module\n");
}

module_init(rbtree_module_init);
module_exit(rbtree_module_cleanup);
MODULE_LICENSE("GPL");