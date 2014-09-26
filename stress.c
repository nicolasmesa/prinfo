#include <stdlib.h>

int main(int argc, char **argv)
{
	int depth = 10;
	while(depth--){
		fork();
		sleep(2);
	}

	while(wait(&depth) != -1)
		;

	return depth;
}
