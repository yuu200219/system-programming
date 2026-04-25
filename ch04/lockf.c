#include <stdio.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char* argv[]) {
	int fd;
	int ret;
	char opt;
	off_t begin, end;
	
	if (argc == 1) printf("lockf [file] [type(l/u)] [begin] [end]\n");

	// fd = open (argv [1], O_WRONLY);
	fd = open(argv[1], O_RDWR);
	printf("fd = %d is opened\n", fd);
	
	sscanf(argv[2], "%c", &opt);
	sscanf(argv[3], "%ld", &begin);
	sscanf(argv[4], "%ld", &end);


	printf("testing lock and unlock\n");
	int len = end - begin + 1;
	// lseek(fd, begin, SEEK_SET);
	// lockf(fd, F_LOCK, len);    // 先加鎖
	// printf("Locked!\n");

	// lseek(fd, begin, SEEK_SET);
	// lockf(fd, F_UNLCK, len);   // 在同一個行程內解鎖
	// printf("Unlocked!\n");
	switch (opt) {
		case 'l':
			lseek(fd, begin, SEEK_SET); // 指向 begin 的位置
			ret = lockf(fd, F_LOCK, end - begin + 1); // 請求一個 exlusive lock，鎖定從 begin 開始的 end - begin + 1 個位元組
			// 如果改範圍已經被其他人 locking，此呼叫會 blocking 直到 lock 被釋放
			break;
		case 'u':
			lseek(fd, begin, SEEK_SET); // 指向 begin 的位置
			ret = lockf(fd, F_UNLCK, end - begin + 1);
			break;
		default:
			printf("input error\n");
	}
	if (ret != 0)
		perror("flock");
	printf("end\n");
	getchar();
	return 0;
}
