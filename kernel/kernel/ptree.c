#include<linux/unistd.h>
#include<linux/sched.h>
#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/prinfo.h>
#include<linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

void add_prinfo(struct prinfo *prinfo_struct, struct task_struct *task) {
	prinfo_struct->pid = task->pid;
	printk(KERN_WARNING "Init child:%p -  %d\n", task, task->pid);
}

struct task_struct * get_first_child(struct task_struct *task)
{
	struct task_struct *temp;
	list_for_each_entry(temp, &task->children, sibling){
		return temp;
	}

	return NULL;
}

void do_dfs(struct task_struct *root, struct prinfo *buf, int max_num, int *curr_num){
	struct task_struct *child, *temp;
	struct list_head list1, list2;
	int found;

	child = root;

	while (1) {
		add_prinfo(buf, child);
		buf++;

		if (get_first_child(child)){
			printk(KERN_WARNING "Different than child\n");
			child = get_first_child(child);
		} else {
			printk(KERN_WARNING "Noooot\n");
			found = 0;
			while (!found) {
				list_for_each_entry(temp, &child->sibling, sibling){
					if (temp == list_entry(&(child->parent->children), struct task_struct, sibling))
						break;

					found = 1;
					break;
				}

				if (child == root)
					return;

				if (!found)
					child = child->parent;
			}

			child = temp;	
		}
	}	

}


SYSCALL_DEFINE2(ptree, struct prinfo *, buf, int *, nr)
{
        int k_int, num = 0, i;
        struct prinfo *k_buf;
	struct list_head *task_list;

        if (buf == NULL || nr == NULL) {
                printk(KERN_WARNING "Either buf or nr are NULL\n");
                return -EINVAL;
        }

        if (copy_from_user(&k_int, nr, sizeof(int))) {
                printk(KERN_WARNING "Error while copying int\n");
                return -EFAULT;
        }

        if (k_int < 1) {
                printk(KERN_WARNING "NR value is less than 1\n");
                return -EINVAL;
        }

        k_buf = kmalloc(sizeof(*k_buf) * k_int, GFP_KERNEL);

        read_lock(&tasklist_lock);

	task_list = &(init_task.children);

	do_dfs(&init_task, k_buf, k_int, &num);

	read_unlock(&tasklist_lock);

	//for (i=0; i < k_int; i++)
	//	printk(KERN_WARNING "PIDD: %d\n", k_buf[i].pid);

        if (copy_to_user(buf, k_buf, k_int * sizeof(* k_buf))) {
                printk(KERN_WARNING "Error while copying buf\n");
                return -EFAULT;
        }


        return num;
}
