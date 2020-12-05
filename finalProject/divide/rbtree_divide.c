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

struct arguments
{
	struct rb_root_cached *root;
	struct my_node *node;
};

static struct rnd_state rnd;

ktime_t t_start, t_end;

int running_thread = 0;
int sequence = 0;

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

static int insert_sync(void *data)
{
	int i;
	struct arguments *args = data;

	running_thread++;
	sequence++;
	for (i = 0; i < 25000; i++) {
		insert(args->node + i, args->root);
	}	
	running_thread--;
	// printk("insert finished pid(%d)\n", current->pid);
	if (sequence == 4) {
		t_end = ktime_get_ns();
		printk("insert(threaded): %lld ns\n", t_end - t_start);
		sequence = 0;
	}
	kthread_stop(current);
	return 0;
}

static int search_sync(void *data)
{
	struct rb_node *node;
	struct arguments *args = data;

	running_thread++;
	sequence++;
	for (node = rb_first(&(args->root->rb_root)); node; node = rb_next(node));
	running_thread--;
	// printk("search finished pid(%d)\n", current->pid);
	if (sequence == 4) {
		t_end = ktime_get_ns();
		printk("search(threaded): %lld ns\n", t_end - t_start);
		sequence = 0;
	}
	kthread_stop(current);
	return 0;	
}

static int erase_sync(void *data)
{
	int i;
	struct arguments *args = data;

	running_thread++;
	sequence++;
	for (i = 0; i < 25000; i++) {
		erase(args->node + i, args->root);
	}
	running_thread--;
	// printk("delete finished pid(%d)\n", current->pid);
	if (sequence == 4) {
		t_end = ktime_get_ns();
		printk("delete(threaded): %lld ns\n", t_end - t_start);
		sequence = 0;
	}
	kthread_stop(current);
	return 0;
}

static void init(void)
{
	int i;
	for (i = 0; i < 100000; i++) {
		if (i < 25000) {
			rbtree[i].value = i;
			rbtree1[i].value = i;
			continue;
		}
		else if (i < 50000) {
			rbtree[i].value = i;
			rbtree2[i].value = i;
			continue;
		}
		else if (i < 75000) {
			rbtree[i].value = i;
			rbtree3[i].value = i;
			continue;
		}
		else if (i < 100000) {
			rbtree[i].value = i;
			rbtree4[i].value = i;
			continue;
		}
	}	
}

int __init rbtree_module_init(void)
{
	int i;
	ktime_t start, end;
	struct rb_node *node;
	struct arguments args1, args2, args3, args4;

	unsigned int n;
	int random;

	rbtree = kmalloc_array(100000, sizeof(*rbtree), GFP_KERNEL);
	rbtree1 = kmalloc_array(25000, sizeof(*rbtree1), GFP_KERNEL);
	rbtree2 = kmalloc_array(25000, sizeof(*rbtree2), GFP_KERNEL);
	rbtree3 = kmalloc_array(25000, sizeof(*rbtree3), GFP_KERNEL);
	rbtree4 = kmalloc_array(25000, sizeof(*rbtree4), GFP_KERNEL);

	prandom_seed_state(&rnd, 3141592653589793238ULL);
	init();

	printk("%d\n", prandom_u32_state(&rnd));
	printk("%d\n", prandom_u32_state(&rnd));

	args1.root = &rbtree1_root; args1.node = rbtree1;
	args2.root = &rbtree2_root; args2.node = rbtree2;
	args3.root = &rbtree3_root; args3.node = rbtree3;
	args4.root = &rbtree4_root; args4.node = rbtree4;

	// printk("\n////////// insert //////////\n");

	start = ktime_get_ns();

	for (i = 0; i < 100000; i++) {
		insert(rbtree + i, &rbtree_root);
	}

	end = ktime_get_ns();
	printk("insert(normal): %lld ns\n", end - start);

	t_start = ktime_get_ns();

	thread1 = kthread_run(insert_sync, &args1, "insert1");
	thread2 = kthread_run(insert_sync, &args2, "insert2");
	thread3 = kthread_run(insert_sync, &args3, "insert3");
	thread4 = kthread_run(insert_sync, &args4, "insert4");

	while (running_thread > 0) {
		msleep(1);
	}


	// printk("\n////////// search //////////\n");

	get_random_bytes(&n, sizeof(int));
	random = n % 100000;
	printk("random value: %d\n", random);

	start = ktime_get_ns();

	for (node = rb_first(&rbtree_root.rb_root); node; node = rb_next(node)) {
		if (rb_entry(node, struct my_node, rb)->value == random){
			end = ktime_get_ns();
			printk("search(normal): %lld ns\n", end - start);	
		}
	}

	// end = ktime_get_ns();
	// printk("search(normal): %lld ns\n", end - start);

	t_start = ktime_get_ns();

	thread1 = kthread_run(search_sync, &args1, "search1");
	thread2 = kthread_run(search_sync, &args2, "search2");
	thread3 = kthread_run(search_sync, &args3, "search3");
	thread4 = kthread_run(search_sync, &args4, "search4");

	while (running_thread > 0) {
		msleep(1);
	}	


	// printk("\n////////// delete //////////\n");

	start = ktime_get();

	for (i = 0; i < 100000; i++) {
		erase(rbtree + i, &rbtree_root);
	}

	end = ktime_get();
	printk("delete(normal): %lld ns\n", end - start);

	t_start = ktime_get_ns();

	thread1 = kthread_run(erase_sync, &args1, "erase1");
	thread2 = kthread_run(erase_sync, &args2, "erase2");
	thread3 = kthread_run(erase_sync, &args3, "erase3");
	thread4 = kthread_run(erase_sync, &args4, "erase4");

	while (running_thread > 0) {
		msleep(1);
	}

	kfree(rbtree);
	kfree(rbtree1);
	kfree(rbtree2);
	kfree(rbtree3);
	kfree(rbtree4);

	printk("finished\n");

	return 0;
}

void __exit rbtree_module_cleanup(void)
{
	printk("\nBye module\n");
}

module_init(rbtree_module_init);
module_exit(rbtree_module_cleanup);
MODULE_LICENSE("GPL");
