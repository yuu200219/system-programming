#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include "common.h"

int main() {
    int fd = open("./shared_lock.data", O_RDWR, 0666);
    if (fd < 0) { perror("請先執行 writer"); exit(1); }

    struct shared_data *sd = mmap(NULL, sizeof(struct shared_data), 
                                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    printf("讀取端已啟動，嘗試讀取資料...\n");
    for(int i = 0; i < 5; i++) {
        printf("嘗試獲取 Lock...\n");
        pthread_mutex_lock(&sd->mutex); // 如果 writer 鎖住了，這裡會 block
        
        printf("讀取成功: [%d] %s\n", sd->count, sd->message);
        
        pthread_mutex_unlock(&sd->mutex);
        sleep(2);
    }

    munmap(sd, sizeof(struct shared_data));
    close(fd);
    return 0;
}