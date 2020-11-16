#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/ktime.h>

struct my_node
{
	struct list_head list;
	int data;
};

void llist_example(void)
{
	struct list_head my_list_1000;
	struct list_head my_list_10000;
	struct list_head my_list_100000;

	struct list_head* p, * pn;
	struct my_node* current_node;
	int i;
	unsigned int n, random;

	ktime_t start;
	ktime_t end;


	INIT_LIST_HEAD(&my_list_1000);
	INIT_LIST_HEAD(&my_list_10000);
	INIT_LIST_HEAD(&my_list_100000);


	////////// Insert //////////
	printk("\n////////// Insert //////////\n");

	/* insert entries at my_list_1000 */
	start = ktime_get();
	for (i = 0; i < 1000; i++) {
		struct my_node* new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
		new->data = i;
		list_add_tail(&new->list, &my_list_1000);
	}
	end = ktime_get();
	printk("It takes 0.%09lld secs to insert 1000 entreis\n", end - start);

	/* insert entries at my_list_10000 */
	start = ktime_get();
	for (i = 0; i < 10000; i++) {
		struct my_node* new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
		new->data = i;
		list_add_tail(&new->list, &my_list_10000);
	}
	end = ktime_get();
	printk("It takes 0.%09lld secs to insert 10000 entreis\n", end - start);

	/* insert entries at my_list_100000 */
	start = ktime_get();
	for (i = 0; i < 100000; i++) {
		struct my_node* new = kmalloc(sizeof(struct my_node), GFP_KERNEL);
		new->data = i;
		list_add_tail(&new->list, &my_list_100000);
	}
	end = ktime_get();
	printk("It takes 0.%09lld secs to insert 100000 entreis\n\n", end - start);


	////////// Search //////////
	printk("////////// Search //////////\n");

	/* search from my_list_1000 */
	get_random_bytes(&n, sizeof(int)); // generate random number
	random = n % 1000;
	printk("random number: %d\n", random);
	start = ktime_get();
	list_for_each_entry(current_node, &my_list_1000, list) {
		if (current_node->data == random) {
			printk("find!\n");
			end = ktime_get();
		}
	}
	printk("It takes 0.%09lld secs to search for %d from 1000 entries\n", end - start, random);

	/* search from my_list_1000 */
	get_random_bytes(&n, sizeof(int)); // generate random number
	random = n % 10000;
	printk("random number: %d\n", random);
	start = ktime_get();
	list_for_each_entry(current_node, &my_list_10000, list) {
		if (current_node->data == random) {
			printk("find!\n");
			end = ktime_get();
		}
	}
	printk("It takes 0.%09lld secs to search for %d from 10000 entries\n", end - start, random);

	/* search from my_list_1000 */
	get_random_bytes(&n, sizeof(int)); // generate random number
	random = n % 100000;
	printk("random number: %d\n", random);
	start = ktime_get();
	list_for_each_entry(current_node, &my_list_100000, list) {
		if (current_node->data == random) {
			printk("find!\n");
			end = ktime_get();
		}
	}
	printk("It takes 0.%09lld secs to search for %d from 100000 entries\n\n", end - start, random);


	////////// Delete //////////
	printk("////////// Delete //////////\n");

	/* delete all entries of my_list_1000 */
	start = ktime_get();
	list_for_each_safe(p, pn, &my_list_1000) {
		current_node = list_entry(p, struct my_node, list);
		list_del(p);
	}
	end = ktime_get();
	printk("It takes 0.%09lld secs to delete 1000 entries\n", end - start);

	/* delete all entries of my_list_10000 */
	start = ktime_get();
	list_for_each_safe(p, pn, &my_list_10000) {
		current_node = list_entry(p, struct my_node, list);
		list_del(p);
	}
	end = ktime_get();
	printk("It takes 0.%09lld secs to delete 10000 entries\n", end - start);

	/* delete all entries of my_list_100000 */
	start = ktime_get();
	list_for_each_safe(p, pn, &my_list_100000) {
		current_node = list_entry(p, struct my_node, list);
		list_del(p);
	}
	end = ktime_get();
	printk("It takes 0.%09lld secs to delete 100000 entries\n\n", end - start);

	kfree(current_node);
}

int __init llist_module_init(void)
{
	printk("llist module init\n");
	llist_example();
	return 0;
}

void __exit llist_module_cleanup(void)
{
	printk("rm llist module\n\n");
}

module_init(llist_module_init);
module_exit(llist_module_cleanup);
MODULE_LICENSE("GPL");
