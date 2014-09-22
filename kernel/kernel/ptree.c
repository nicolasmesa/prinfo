#include<linux/unistd.h>
#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/prinfo.h>
#include<linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

SYSCALL_DEFINE2(ptree, struct prinfo *, buf, int *, nr)
{
        int k_int, num = 0;
        struct prinfo *k_buf;
        struct task_struct *task, *parent;

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

        for_each_process(task) {
                if (num < k_int) {
                        parent = task->parent;

                        if (parent != NULL)
                                (buf + num)->parent_pid = parent->pid;
                        else
                                (buf + num)->parent_pid = 0;

                        (buf + num)->pid = task->pid;
                }

                num++;
        }

        read_unlock(&tasklist_lock);


        if (copy_to_user(buf, k_buf, k_int)) {
                printk(KERN_WARNING "Error while copying buf\n");
                return -EFAULT;
        }


        return num;
}
