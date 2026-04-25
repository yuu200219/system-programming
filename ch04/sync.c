#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char** argv) {
	int fd;
	int num;
	int printFileInfo=0;
	printf("『%s print』即時列印檔案訊息，不加參數則會列印\n", argv[0]);
	if (argc == 1) {
		printf("不列印檔案訊息\n");
		printFileInfo = 0;
	} else {
		printf("列印檔案訊息\n");
		printFileInfo = 1;
	}
	printf("按下enter後繼續\n");
	getchar();

	fd = open("./syncFile.data",O_WRONLY | O_CREAT | O_TRUNC, 0644);
	for(num=0; num <=100000; num++) {
		write(fd, "1234", sizeof("1234"));
		fsync(fd); // 將資料立即同步到 disk
		if (num%10000==1) {
			write(1, "*", sizeof("*")); // 1 是標準輸出 (Standard Output) 的檔案描述符
			fsync(1); // "*" 立即同步到螢幕上，否則可能會等到緩衝區滿了才印出來
		}
		if (num%100==0 && printFileInfo==1)
			system("ls syncFile.data -alhs");
	}
	return 0;
}
