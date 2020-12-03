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
	bool islocked;
	pid_t pid;
};

bool node_is_locked(struct my_node *node)
{
	return node->islocked;
}

bool unlock_node(struct my_node *node)
{
	if (current->pid == node->pid) {
		node->islocked = false;
		return true;
	}
	return false;
}

bool lock_node(struct my_node *node)
{
	if (node->islocked == false) {
		node->islocked = true;
		node->pid = current->pid;
		return true;
	}
	return false;
}

int a;

static struct rb_root_cached rbtree_100000_root = RB_ROOT_CACHED;

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

static int erase_sync(struct my_node *node, struct rb_root_cached *root)
{
	if (node->islocked) { // if node is locked
		return -1;
	}
	lock_node(node);
	rb_erase(&node->rb, &root->rb_root);
	unlock_node(node);
}

static void init(void)
{
	int i;
	for (i = 0; i < 100000; i++) {
			rbtree_100000[i].value = prandom_u32_state(&rnd);
	}	
}

void RB_example(void)
{	
	int i;
	ktime_t start, end;
	struct rb_node *node;

	rbtree_100000 = kmalloc_array(100000, sizeof(*rbtree_100000), GFP_KERNEL);

	prandom_seed_state(&rnd, 3141592653589793238ULL);
	init();

	////////// insert //////////
	// printk("\n////////// insert //////////\n");

	/* insert 100000 entries */
	start = ktime_get_ns();

	for (i = 0; i < 100000; i++) {
		insert(rbtree_100000 + i, &rbtree_100000_root);
	}
	
	end = ktime_get_ns();
	
	if (a == 0) {
		printk("Spinlock linked list insert time : %lld ns\n", end - start);
	}
	else if (a == 1) {
		printk("Mutex linked list insert time : %lld ns\n", end - start);
	}
	else if (a == 2) {
		printk("RW Semaphore linked list insert time : %lld ns\n", end - start);
	}

	////////// traverse //////////
	// printk("\n////////// search //////////\n");

	/* traverse 100000 entries */
	start = ktime_get_ns();

	for (node = rb_first(&rbtree_100000_root.rb_root); node; node = rb_next(node));

	end = ktime_get_ns();

	if (a == 0) {
		printk("Spinlock linked list search time : %lld ns\n", end - start);
	}
	else if (a == 1) {
		printk("Mutex linked list search time : %lld ns\n", end - start);
	}
	else if (a == 2) {
		printk("RW Semaphore linked list search time : %lld ns\n", end - start);
	}

	////////// delete //////////
	// printk("\n////////// delete //////////\n");

	/* delete 100000 entries */
	start = ktime_get();

	for (i = 0; i < 100000; i++) {
		erase(rbtree_100000 + i, &rbtree_100000_root);
	}

	end = ktime_get();

	if (a == 0) {
		printk("Spinlock linked list delete time : %lld ns\n", end - start);
	}
	else if (a == 1) {
		printk("Mutex linked list delete time : %lld ns\n", end - start);
	}
	else if (a == 2) {
		printk("RW Semaphore linked list delete time : %lld ns\n", end - start);
	}
}

int __init rbtree_module_init(void)
{
	for (a = 0; a < 3; a++) {
		RB_example();
		printk("\n");
	}

	return 0;
}

void __exit rbtree_module_cleanup(void)
{
	printk("\nBye module\n");
}

module_init(rbtree_module_init);
module_exit(rbtree_module_cleanup);
MODULE_LICENSE("GPL");
