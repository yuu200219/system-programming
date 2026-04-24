#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    const char *filepath = "./shared_mmap.data";
    size_t size = 4096; // 映射一個 Page 的大小

    // 1. 建立並初始化檔案大小
    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, 0666);
    ftruncate(fd, size); // 直接將檔案撐開到 4KB

    // 2. 使用 MAP_SHARED 進行映射
    // 注意：必須是 MAP_SHARED 才能讓父子行程看到彼此
    char *shared_mem = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    if (shared_mem == MAP_FAILED) {
        perror("mmap 失敗");
        return 1;
    }

    // 先在記憶體寫入初始值
    strcpy(shared_mem, "Initial message from Parent");

    printf("--- 初始內容: %s ---\n", shared_mem);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork 失敗");
        return 1;
    }

    if (pid == 0) { // 子行程
        printf("子行程 (PID: %d) 正在讀取內容...\n", getpid());
        sleep(1); // 等待父行程修改
        printf("子行程讀取到父行程的修改: %s\n", shared_mem);

        printf("子行程正在寫入新資料...\n");
        sprintf(shared_mem, "Hello Parent, I am Child (PID: %d)", getpid());
        
        munmap(shared_mem, size);
        exit(0);
    } else { // 父行程
        printf("父行程 (PID: %d) 正在修改內容...\n", getpid());
        sprintf(shared_mem, "Parent (PID: %d) updated this!");

        wait(NULL); // 等待子行程執行完畢
        printf("父行程在子行程結束後，讀取到的最終內容: %s\n", shared_mem);

        // 清理資源
        munmap(shared_mem, size);
        close(fd);
    }

    return 0;
}