#include<linux/unistd.h>
#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/prinfo.h>
#include<linux/syscalls.h>
SYSCALL_DEFINE0(ptree)
{
	printk(KERN_WARNING "This is my first syscall!\n");
	return 100;
}
