#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/rbtree.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <asm/timex.h>
#include <linux/ktime.h>

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
	cycles_t time1, time2;
	ktime_t start, end;
	struct rb_node *node;

	rbtree_1000 = kmalloc_array(1000, sizeof(*rbtree_1000), GFP_KERNEL);
	rbtree_10000 = kmalloc_array(10000, sizeof(*rbtree_10000), GFP_KERNEL);
	rbtree_100000 = kmalloc_array(100000, sizeof(*rbtree_100000), GFP_KERNEL);

	printk("\n********** rbtree testing!! **********\n");

	prandom_seed_state(&rnd, 3141592653589793238ULL);
	init();

	////////// insert //////////
	printk("\n////////// insert //////////\n");

	/* insert 1000 entries */
	time1 = get_cycles();
	start = ktime_get();

	for (i = 0; i < 1000; i++) {
		insert(rbtree_1000 + i, &rbtree_1000_root);
	}
	
	time2 = get_cycles();
	end = ktime_get();

	printk("insert(1000 entries): 0.%09lld secs\n", end - start);
	// printk("insert(1000 entries): %llu cycles\n", (unsigned long long)(time2 - time1));

	/* insert 10000 entries */
	time1 = get_cycles();
	start = ktime_get();

	for (i = 0; i < 10000; i++) {
		insert(rbtree_10000 + i, &rbtree_10000_root);
	}
	
	time2 = get_cycles();
	end = ktime_get();

	printk("insert(10000 entries): 0.%09lld secs\n", end - start);
	// printk("insert(10000 entries): %llu cycles\n", (unsigned long long)(time2 - time1));

	/* insert 100000 entries */
	time1 = get_cycles();
	start = ktime_get();

	for (i = 0; i < 100000; i++) {
		insert(rbtree_100000 + i, &rbtree_100000_root);
	}
	
	time2 = get_cycles();
	end = ktime_get();

	printk("insert(100000 entries): 0.%09lld secs\n", end - start);
	// printk("insert(100000 entries): %llu cycles\n", (unsigned long long)(time2 - time1));

	////////// traverse //////////
	printk("\n////////// search //////////\n");

	/* traverse 1000 entries */
	time1 = get_cycles();
	start = ktime_get();

	for (node = rb_first(&rbtree_1000_root.rb_root); node; node = rb_next(node));

	time2 = get_cycles();
	end = ktime_get();

	printk("search(1000 entries): 0.%09lld secs\n", end - start);
	// printk("traverse(1000 entries): %llu cycles\n", (unsigned long long)(time2 - time1));

	/* traverse 10000 entries */
	time1 = get_cycles();
	start = ktime_get();

	for (node = rb_first(&rbtree_10000_root.rb_root); node; node = rb_next(node));

	time2 = get_cycles();
	end = ktime_get();

	printk("search(10000 entries): 0.%09lld secs\n", end - start);
	// printk("traverse(10000 entries): %llu cycles\n", (unsigned long long)(time2 - time1));

	/* traverse 100000 entries */
	time1 = get_cycles();
	start = ktime_get();

	for (node = rb_first(&rbtree_100000_root.rb_root); node; node = rb_next(node));

	time2 = get_cycles();
	end = ktime_get();

	printk("search(100000 entries): 0.%09lld secs\n", end - start);
	// printk("traverse(100000 entries): %llu cycles\n", (unsigned long long)(time2 - time1));		

	////////// delete //////////
	printk("\n////////// delete //////////\n");

	/* delete 1000 entries */
	time1 = get_cycles();
	start = ktime_get();

	for (i = 0; i < 1000; i++) {
		erase(rbtree_1000 + i, &rbtree_1000_root);
	}

	time2 = get_cycles();
	end = ktime_get();

	printk("delete(1000 entries): 0.%09lld secs\n", end - start);
	// printk("erase(1000 entries): %llu cycles\n", (unsigned long long)(time2 - time1));

	/* delete 10000 entries */
	time1 = get_cycles();
	start = ktime_get();

	for (i = 0; i < 10000; i++) {
		erase(rbtree_10000 + i, &rbtree_10000_root);
	}

	time2 = get_cycles();
	end = ktime_get();

	printk("delete(10000 entries): 0.%09lld secs\n", end - start);
	// printk("erase(10000 entries): %llu cycles\n", (unsigned long long)(time2 - time1));

	/* delete 100000 entries */
	time1 = get_cycles();
	start = ktime_get();

	for (i = 0; i < 100000; i++) {
		erase(rbtree_100000 + i, &rbtree_100000_root);
	}

	time2 = get_cycles();
	end = ktime_get();

	printk("delete(100000 entries): 0.%09lld secs\n", end - start);
	// printk("delete(100000 entries): %llu cycles\n", (unsigned long long)(time2 - time1));
}

int __init rbtree_module_init(void)
{
	printk("module init\n");
	RB_example();

	return 0;
}

void __exit rbtree_module_cleanup(void)
{
	printk("\nBye module\n");
}

module_init(rbtree_module_init);
module_exit(rbtree_module_cleanup);
MODULE_LICENSE("GPL");
