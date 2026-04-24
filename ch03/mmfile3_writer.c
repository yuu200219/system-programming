#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

int main() {
    int fd = open("./shared_lock.data", O_RDWR | O_CREAT | O_TRUNC, 0666);
    ftruncate(fd, sizeof(struct shared_data));

    struct shared_data *sd = mmap(NULL, sizeof(struct shared_data), 
                                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // --- 關鍵步驟：初始化跨行程 Mutex ---
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); // 設定為行程間共享
    pthread_mutex_init(&sd->mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    sd->count = 0;

    printf("寫入端已啟動，準備進入迴圈...\n");
    for(int i = 0; i < 5; i++) {
        pthread_mutex_lock(&sd->mutex); // 上鎖
        
        sd->count++;
        sprintf(sd->message, "這是寫入端第 %d 次更新", sd->count);
        printf("已寫入: %s\n", sd->message);
        
        sleep(3); // 模擬耗時運算，此時讀取端會被擋在門外
        
        pthread_mutex_unlock(&sd->mutex); // 解鎖
        sleep(1);
    }

    munmap(sd, sizeof(struct shared_data));
    close(fd);
    return 0;
}