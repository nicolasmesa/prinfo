#include<sys/syscall.h>
#include<unistd.h>

int main(int argc, char **argv) {
	syscall(223);
	return 0;
}
