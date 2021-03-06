#include <unistd.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
struct prinfo {
	pid_t parent_pid;               /* process id of parent */
	pid_t pid;                      /* process id */
	pid_t first_child_pid;          /* pid of youngest child */
	pid_t next_sibling_pid;         /* pid of older sibling */
	long state;                     /* current state of process */
	long uid;                       /* user id of process owner */
	char comm[64];                  /* name of program executed */
};

int ptree(struct prinfo *buf, int *nr)
{
	return syscall(223, buf, nr);
}

void print_tabs(int count)
{
	while (count--)
		printf("\t");
}

void print_tree(struct prinfo *buff, int nr)
{
	int i, curr_level = 0, *levels;
	int curr_parent, prev_parent, prev_child;

	levels = malloc(sizeof(int) * nr);

	if (levels == NULL) {
		printf("Error: %s", strerror(errno));
		exit(-1);
	}

	levels[0] = buff[0].pid;
	prev_parent = -1;
	prev_child = 0;

	printf("%s,%d,%ld,%d,%d,%d,%lu\n", buff[0].comm, buff[0].pid,
		buff[0].state, buff[0].parent_pid,
		buff[0].first_child_pid, buff[0].next_sibling_pid, buff[0].uid);

	for (i = 1; i < nr; i++) {
		curr_parent = buff[i].parent_pid;

		if (curr_parent == prev_child) {
			curr_level++;
		} else if (curr_parent != prev_parent) {
			while (curr_level >= 0 && levels[curr_level]
							!= curr_parent)
				curr_level--;

			curr_level++;
		}

		levels[curr_level] = buff[i].pid;

		prev_child = buff[i].pid;
		prev_parent = curr_parent;

		print_tabs(curr_level);
		printf("%s,%d,%ld,%d,%d,%d,%lu\n", buff[i].comm, buff[i].pid,
			buff[0].state, buff[i].parent_pid,
			buff[i].first_child_pid,
			buff[i].next_sibling_pid, buff[i].uid);
	}

	free(levels);
}

int main(int argc, char **argv)
{
	int print_flag = 0;

	if (argc != 3) {
		printf("Usage: ./prinfo <num of processes requested>
			--r/--x\n");
		return 0;
	}
	if (strcmp(argv[2], "--r") == 0)
		print_flag = 1;

	else if (strcmp(argv[2], "--x") == 0)
		print_flag = 0;
	else {

		printf("Usage: ./prinfo <num of processes requested>
				--r/--x\n");
		return 0;
	}

	int nr = atoi(argv[1]);

	if (nr < 1) {
		printf("Error: The number of processes has to be
				 greater than 0\n");
		exit(-1);
	}

	struct prinfo *buf = malloc(sizeof(struct prinfo) * nr);

	if (!buf) {
		printf("Error: %s\n", strerror(errno));
		exit(-1);
	}

	int ret;

	ret = ptree(buf, &nr);
	ret = ret < nr ? ret : nr;
	print_tree(buf, ret);

	if (print_flag == 1)
		printf("\nNum of processes returned is %d\n", ret);

	free(buf);

	return 0;
}
