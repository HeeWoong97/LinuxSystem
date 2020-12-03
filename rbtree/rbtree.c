#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <asm/timex.h>
#include <linux/ktime.h>
#include <linux/sched.h>

struct my_node
{
	int value;
	struct rb_node rb;
};

static struct rb_root_cached rbtree_1000_root = RB_ROOT_CACHED;
static struct rb_root_cached rbtree_10000_root = RB_ROOT_CACHED;
static struct rb_root_cached rbtree_100000_root = RB_ROOT_CACHED;

static struct my_node *rbtree_1000 = NULL;
static struct my_node *rbtree_10000 = NULL;
static struct my_node *rbtree_100000 = NULL;

static struct rnd_state rnd;

struct rb_node *rb_next_sync(const struct rb_node *node)
{
	struct rb_node *parent;

	if (RB_EMPTY_NODE(node))
		return NULL;

	/*
	 * If we have a right-hand child, go down and then left as far
	 * as we can.
	 */
	if (node->rb_right) {
		node = node->rb_right;
		while (node->rb_left)
			node = node->rb_left;
		return (struct rb_node *)node;
	}

	/*
	 * No right-hand children. Everything down and left is smaller than us,
	 * so any 'next' node must be in the general direction of our parent.
	 * Go up the tree; any time the ancestor is a right-hand child of its
	 * parent, keep going up. First time it's a left-hand child of its
	 * parent, said parent is our 'next' node.
	 */
	while ((parent = rb_parent(node)) && node == parent->rb_right)
		node = parent;

	return parent;
}

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
}

int __init rbtree_module_init(void)
{
	struct task_struct *result = pid_task(find_vpid((int) task_pid_nr(current)), PIDTYPE_PID);
	ktime_t start, end;
	int i;
	
	start = ktime_get();

	printk("\n********** rbtree testing!! **********\n");
	printk("scheduling policy: %d\n", result->policy);
	printk("first vruntime: %lld\n", result->se.vruntime);

	for (i = 0; i < 10; i++) {
		// printk("\n////////// loop %d //////////\n", i + 1);
		RB_example();
		// printk("\n");
	}

	printk("second vruntime: %lld\n\n", result->se.vruntime);

	end = ktime_get();
	printk("total time: 0.%09lld secs\n", end - start);

	return 0;
}

void __exit rbtree_module_cleanup(void)
{
	printk("\nBye module\n");
}

module_init(rbtree_module_init);
module_exit(rbtree_module_cleanup);
MODULE_LICENSE("GPL");
