#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

int main() {
	int fd;
	const int G=1024*1024*1024; // 1G
	const int M=1024*1024;

	printf("這個程式會在當前的目錄下，製造檔案myHole\n");
	fd = open("./myHole", O_RDWR| O_CREAT | O_TRUNC, S_IRUSR| S_IWUSR);
	if (fd <0)
		perror("無法製造myHole");
	//lseek將『檔案指標』由開始位置向後移動10M，lseek比較可能出錯，用assert檢查一下
    assert(lseek(fd, 10*M, SEEK_SET) != -1); // SEEK_SET 從開始位置往後
	//寫入“1”，很少出錯，懶得檢查
	write(fd, "1", sizeof("1"));
	//lseek將『檔案指標』由『目前』位置向後移動10M，lseek比較可能出錯，用assert檢查一下
	assert(lseek(fd, 10, SEEK_CUR) != -1); // SEEK_CUR 目前為直往後
	write(fd, "2", sizeof("2"));
	assert(lseek(fd, 10, SEEK_CUR) != -1);
	write(fd, "3\n", sizeof("3\n"));
	close(fd);
	system("ls myHole -alhs");
	// 4.0K -rw------- 1 yuu yuu 9.6M Apr 24 15:35 myHole, -l: 邏輯大小(9.6M), -h: 人類可讀的大小, -s: 實際佔用的大小(4k)
	// 實際上 myHole 佔用的磁碟空間是 4K，但檔案大小是 9.6M，這就是 hole 的特性	
	return 0;
}
