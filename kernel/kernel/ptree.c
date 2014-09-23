#include<linux/unistd.h>
#include<linux/sched.h>
#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/prinfo.h>
#include<linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

void do_dfs(struct list_head *head, struct prinfo *buf, int max_num, int *curr_num){
	struct task_struct *child;

	list_for_each_entry(child, head, sibling) {
                //printk(KERN_WARNING "Init child:%p -  %d\n", child, child->pid);

		if((*curr_num) < max_num) {
		//	printk(KERN_WARNING "Yes %d/%d\n", *curr_num, max_num);
			if (child->parent != NULL)
                                (buf + (*curr_num))->parent_pid = child->parent->pid;
                        else
                                (buf + (*curr_num))->parent_pid = 0;

                        (buf +*(curr_num))->pid = child->pid;

		}

		(*curr_num)++;
		do_dfs(&child->children, buf, max_num, curr_num);
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

	do_dfs(task_list, k_buf, k_int, &num);

	read_unlock(&tasklist_lock);

	//for (i=0; i < k_int; i++)
	//	printk(KERN_WARNING "PIDD: %d\n", k_buf[i].pid);

        if (copy_to_user(buf, k_buf, k_int * sizeof(* k_buf))) {
                printk(KERN_WARNING "Error while copying buf\n");
                return -EFAULT;
        }


        return num;
}
