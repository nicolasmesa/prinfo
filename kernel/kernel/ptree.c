#include<linux/unistd.h>
#include<linux/sched.h>
#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/prinfo.h>
#include<linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

/*add_prinfo is used to gather the information needed to populate
the prinfo structures. even here we ensure that threads are not
added as children or siblings by doing the thread group leader
check*/

void add_prinfo(struct prinfo *prinfo_struct, struct task_struct *task)
{
	struct task_struct *child;
	struct task_struct *sibling;
	struct list_head *pos;	
	struct task_struct *temp;

	if (list_empty(&task->children))
		child = NULL;
	else
		child = list_first_entry(&task->children,
				struct task_struct, sibling);
	
	list_for_each(pos, &(task->parent->children)) {

	temp = list_entry(pos, struct task_struct, sibling);
	if(temp->pid == task->pid) {
			
		sibling = list_entry(pos->prev, struct task_struct, sibling);
		break;
		
		}

	}		

	if (sibling == list_entry(&(task->parent->children),
					struct task_struct, sibling)) {
		printk(KERN_DEBUG "Equal\n");
		sibling = NULL;
	}

	prinfo_struct->pid = task->pid;
	prinfo_struct->parent_pid = task->parent->pid;

	if (child && thread_group_leader(child))
		prinfo_struct->first_child_pid = child->pid;
	else
		prinfo_struct->first_child_pid = 0;

	if (sibling && thread_group_leader(sibling))
		prinfo_struct->next_sibling_pid = sibling->pid;
	else
		prinfo_struct->next_sibling_pid = 0;

	prinfo_struct->state = task->state;

	prinfo_struct->uid = task->cred->uid;

	get_task_comm(prinfo_struct->comm, task);
}

/*Self explanatory function. Wrote it as a wrapper*/

struct task_struct *get_first_child(struct task_struct *task)
{
	struct task_struct *temp;

	list_for_each_entry(temp, &task->children, sibling) {
		return temp;
	}

	return NULL;
}

/*do_dfs stores the process information in the dfs order. It
checks to ensure that threads are not being added to the tree
by adding only thread group leaders. Algorithm is an iterative
version of the depth first search algorithm. We keep track of
the children of the process by using the found flag and otherwise
iterate through the sibling list*/

int do_dfs(struct task_struct *root,
	struct prinfo *buf, int max_num)
{

	struct task_struct *child, *temp;
	int found;
	int actual_num = 0;

	child = root;

	while (1) {

		if (actual_num < max_num) {
			if (thread_group_leader(child)) {
				add_prinfo(buf, child);
				buf++;
			} else
				actual_num--;
		}
		if (get_first_child(child))
			child = get_first_child(child);
		else {
			found = 0;
			while (!found) {
				list_for_each_entry(temp,
					&child->sibling, sibling) {

					if (temp == list_entry(
		&(child->parent->children), struct task_struct, sibling))
						break;

					found = 1;
					break;
				}

				if (child == root)
					return actual_num;

				if (!found)
					child = child->parent;
			}

			child = temp;
		}

		actual_num++;
	}
	return actual_num;
}

/*The main system call. Wrapper for this is ptree() written in the
test program. After initial error checking, alloc kernel memory and
then populate it using the function add_prinfo above. No allocation
or user spae copy is done when the locks are held. do_dfs returns
the actual number of processes running but copies information only
for that number requested. The return value is passed back up to user
space.*/

SYSCALL_DEFINE2(ptree, struct prinfo __user *, buf, int __user *, nr)
{
	int k_int, copy_size;
	struct prinfo *k_buf;
	struct list_head *task_list;
	int ret_val = 0;

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

	k_buf = kmalloc_array(k_int, sizeof(*k_buf), GFP_KERNEL);

	if (k_buf == NULL)
		return -ENOMEM;

	read_lock(&tasklist_lock);

	task_list = &(init_task.children);
	ret_val = do_dfs(&init_task, k_buf, k_int);
	read_unlock(&tasklist_lock);

	copy_size = ret_val < k_int ? ret_val : k_int;

	if (copy_to_user(buf, k_buf, copy_size * sizeof(*k_buf))) {
		printk(KERN_WARNING "Error while copying buf\n");
		return -EFAULT;
	}

	kfree(k_buf);

	return ret_val;
}
