#include <unistd.h>
#include <stdio.h>
#include <sys/syscall.h>

#define NR 100 

struct prinfo {
        pid_t parent_pid;               /* process id of parent */
        pid_t pid;                      /* process id */
        pid_t first_child_pid;          /* pid of youngest child */
        pid_t next_sibling_pid;         /* pid of older sibling */
        long state;                     /* current state of process */
        long uid;                       /* user id of process owner */
        char comm[64];                  /* name of program executed */
};

int ptree(struct prinfo* buf, int* nr)
{
	return syscall(223, buf, nr);
}

void print_tabs(int count)
{
	while(count--)
		printf("\t");
}

void print_tree(struct prinfo *buff, int nr)
{
	int i, curr_level = 0, levels[100];
	int curr_parent, prev_parent, prev_child;

	levels[0] = buff[0].pid;
	prev_parent = -1;
	prev_child = 0;

	printf("Parent task id: %d\tTask id %d\t Command: %s\n", buff[0].parent_pid, buff[0].pid, buff[0].comm);

	for (i = 1; i < nr; i++) {
		curr_parent = buff[i].parent_pid;

		if (curr_parent == prev_child) {
			curr_level++;
			levels[curr_level] = buff[i].pid;
			
		}else if (curr_parent != prev_parent) {
			while (levels[curr_level] != curr_parent)
				curr_level--;

			curr_level++;
			levels[curr_level] = buff[i].pid;
		}

		prev_child = buff[i].pid;
		prev_parent = curr_parent;

		print_tabs(curr_level);
		printf("Parent task id: %d\tTask id %d\t Command: %s\n", buff[i].parent_pid, buff[i].pid, buff[i].comm);
	}
}

int main(int argc, char **argv) {
	struct prinfo buf[NR], temp;
	int nr = NR, ret, i;

	ret = ptree( buf, &nr);	
	printf("Returned: %d\n", ret);

	ret = ret < nr ? ret : nr;

	for (i = 0; i < ret; i++) {
		temp = buf[i];
		printf("Parent task id: %d\tTask id %d\t Command: %s\n", buf[i].parent_pid, buf[i].pid, buf[i].comm);
	}	

	print_tree(buf, ret);

	return 0;
}
