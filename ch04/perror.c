#include <stdio.h>
#include <errno.h>

int main() {
	errno=104;
	perror("the error is");
}
