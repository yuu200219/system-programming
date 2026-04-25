## `flock.c`
`file_descriptor = open(argv[1], O_WRONLY);` 一般來說 0, 1, 2 分別是給 stdin, stdout, stderr。因此我們呼叫的 `open` 一般的 file descriptor 會是 3 起跳。
之後選擇輸入 e, s, u，分別代表 exclusive lock, shared lock, unlock。exlusive lock 是唯讀寫，只要進去就沒人可以讀寫。shared lock 是唯讀，大家都可以進來一起讀，但不行有人寫。unlock 就是釋放鎖，一般可以用在同一個 process 在 lock 之後去釋放，如果你直接呼叫 `flock(UN_LOCK)` 他會直接通過，因為當前這個 process 並沒有對這個檔案進行 lock。因此可以實驗 e -> e -> s, s -> e -> s。可以發現 e 會卡任何人的進入;s 只會通行 s，但會擋住 e。
## `hole.c`
- `fd = open("./myHole", O_RDWR| O_CREAT | O_TRUNC, S_IRUSR| S_IWUSR);` `O_RDWR` 代表讀寫模式，`O_CREAET` 代表如果沒有檔案就創建，`O_TRUNC` 如果存在將其長度截斷為0，`S_IRUSER|S_IWUSER` 設定權限為擁有者可讀寫。
- `lseek(fd, 10*M, SEEK_SET)`: 從檔案開頭移動 10MB
- `lseek(fd, 10, SEEK_CUR)`: 從檔案目前位置移動 10 bytes
- `system("ls myHole -alhs")`: 驗證結果，`-l` 會顯示邏輯大小，`-s` 會顯示實際佔磁碟大小，`-h` 讓人類看得懂的單位
## `lockf.c`
- 展示了如何使用 `lockf` 系統呼叫來對檔案的「特定位元組範圍」進行 記錄鎖定 (Record Locking)
- 跟 flock 一樣，lockf unlock 只能針對同一個 process 進行 unlock
- 如果目前 file 的某段 byte 被別人 lock，只要其他人想要對某一段 byte 進行 lock，都要 blocking 等到拿到 lock 的那個人釋放，才能進行後續額外的 lock
## `mmap_cp.c`
- 這邊主要是練習 透過 `mmap` 與 `memcpy` 來達成檔案內容複製
- `mmap` 就是將實體記憶體映射到虛擬記憶體，並對虛擬記憶體進行操作，達成檔案內容複製。
- `ftruncate(fd, length)` 是用於將已開啟的檔案大小調整為指定長度 length。注意 `ftruncate` 不會 改變檔案的當前讀寫偏移量。若要徹底清空檔案並重新寫入，通常需要配合 lseek(fd, 0, SEEK_SET) 將指標重置到檔案開頭，否則直接寫入可能會從中間甚至舊內容之後開始

## need to know befor the `sync.c`
- 現在有三種 sync 方式: `sync, fsync, fdatasync`
- 一般來說都會有 buffer 或是 page cache 會把寫入的資料寫到這裡，之後會進入到 queue 慢慢寫回到 disk，但這就有同步問題，為了保持 file system 跟 disk 的同步性才會有這些同步 function。
- `sync` 只是單純的把資料寫入到 queue，他是週期性每 30 秒執行，不會等 disk 寫完才 return
- `fsync` 則是針對單一的 fd，他會等 disk 寫完後才 return。fsync 常用在 database application，需確保資料真的有寫入到 disk。
- `fdatasync` 跟 fsync 類似，但他只針對 data portion of file，不是整個 file 都會丟到 queue。

## `sync.c`
```
for(num=0; num <=100000; num++) {
    write(fd, "1234", sizeof("1234"));
    fsync(fd);
    ...
}
```
- 在迴圈中寫入 100,001 次 "1234"
- `fsync(fd)` 的作用
    - 通常 Linux 的 write 只是把資料寫入核心的 Page Cache (緩衝區)，並不會立刻寫入實體磁碟（為了效能）。
    - fsync 會強制要求作業系統將檔案緩衝區的所有資料以及元數據 (Metadata, 如檔案大小、修改時間) 立即同步寫入磁碟硬體。 
    - 效能代價：每次寫入都呼叫 fsync 會導致程式速度極慢。因為磁碟 I/O 是最慢的硬體操作，這會強制 CPU 等待磁碟回應（I/O Wait）
- 進度顯示與 `fsync(1)`
    ```
    if (num%10000==1) {
        write(1, "*", sizeof("*"));
        fsync(1);
    }
    ```
    - `write(1...)`: 1 代表示 fd = 1，也就是 stdout
    - `fsync(1)`: 馬上輸出 * 到 stdout
- 同理在 `syncDataOnly.c` 實驗 `fdatasync` 跟 `syncNone.c`，`syncNone.c` 跑最快，他是直接寫入到 buffer 後就不等 disk。
## `vio.c`
- vio 是指 vI/O or vector I/O
    - vio 的 struct 是這樣定義
        ```
        struct iovec {
            void  *iov_base;    /* Starting address */
            size_t iov_len;     /* Number of bytes to transfer */
        };
        ```
- linux 有個機制叫做 scatter-gather I/O，`writev` 就是實作核心
- `writev`
    - 一般的 `write` 只能一次從一個連續的記憶體緩衝區 (buffer) 寫入資料，如果你有三段不同的資料分散在三個記憶體：
        - `write` 三次，但是會呼叫三次 system call，效能差
        - 使用 `memcpy`，開一個大的 buffer 把三段 copy 進去，但是很浪費空間且 CPU resource
    - vector write (`writev`) 解決了這個問題，他允許你傳一個 `iovec`，kernel 會自動幫你把這些不連續的記憶體區塊聚集起來，透過一次 system call 發送出去。
