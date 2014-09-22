#include <unistd.h>
#include <stdio.h>
#include <sys/syscall.h>

#define NR 50

struct prinfo {
        pid_t parent_pid;               /* process id of parent */
        pid_t pid;                      /* process id */
        pid_t first_child_pid;          /* pid of youngest child */
        pid_t next_sibling_pid;         /* pid of older sibling */
        long state;                     /* current state of process */
        long uid;                       /* user id of process owner */
        char comm[64];                  /* name of program executed */
};

int main(int argc, char **argv) {
	struct prinfo buf[NR], temp;
	int nr = NR, ret, i;

	ret = syscall(223, buf, &nr);	
	printf("Returned: %d\n", ret);

	ret = ret < nr ? ret : nr;

	for (i = 0; i < ret; i++) {
		temp = buf[i];
		printf("Parent task id: %d\tTask id %d\n", temp.parent_pid, temp.pid);
	}	

	return 0;
}
