#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

int main() {
	int i;
	char* p1;
	char* p2;
	char* p3;
	char command[1024];
	printf("執行malloc『前』的memory的layout\n");
	sprintf(command, "pmap %d", getpid());
	assert(system(command) >=0);
	printf("pid = %d\n", getpid());
	printf("malloc 256K\n");
	p1=(char*)malloc(64*4096);
	printf("malloc(1)\n");
	p2=(char*)malloc(1);
    printf("malloc(1)\n");
    p3=(char*)malloc(1);
	printf("執行malloc『後』的memory的layout\n");
	sprintf(command, "pmap %d", getpid());
	assert(system(command) >=0);
}
