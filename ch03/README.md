## malloc.c
實驗 `malloc` 會如何配置記憶體。
以某次的輸出結果為例：
```
pid = 55848
malloc(64)
p1=0x6249aceb06b0 // p1 起始位址
malloc 64*4K
p2=0x7a71e4e9b010 // p2 起始位址
55848:   ./malloc
000062497332c000      4K r---- malloc
000062497332d000      4K r-x-- malloc
000062497332e000      4K r---- malloc
000062497332f000      4K r---- malloc
0000624973330000      4K rw--- malloc
00006249aceb0000    132K rw---   [ anon ] // p1 位址對應到這裡
00007a71e4c00000    160K r---- libc.so.6
00007a71e4c28000   1568K r-x-- libc.so.6
00007a71e4db0000    316K r---- libc.so.6
00007a71e4dff000     16K r---- libc.so.6
00007a71e4e03000      8K rw--- libc.so.6
00007a71e4e05000     52K rw---   [ anon ]
00007a71e4e9b000    272K rw---   [ anon ] // P2 位址對應到這裡
00007a71e4eef000      8K rw---   [ anon ]
00007a71e4ef1000     16K r----   [ anon ]
00007a71e4ef5000      8K r----   [ anon ]
00007a71e4ef7000      8K r-x--   [ anon ]
00007a71e4ef9000      4K r---- ld-linux-x86-64.so.2
00007a71e4efa000    172K r-x-- ld-linux-x86-64.so.2
00007a71e4f25000     40K r---- ld-linux-x86-64.so.2
00007a71e4f2f000      8K r---- ld-linux-x86-64.so.2
00007a71e4f31000      8K rw--- ld-linux-x86-64.so.2
00007ffc97942000    136K rw---   [ stack ]
ffffffffff600000      4K --x--   [ anon ]
 total             2956K
```
發現有趣的現象：
- p1(64bytes)
  - p1 明明只被分配到 64bytes malloc 卻給了 132K
  - 這是因為 malloc 會呼叫 `brk()` 來堆高 heap 的邊界 
  - 這是因為 malloc 為了效率，會預先向系統要一大塊地（Arena），再慢慢分給你。
- p2(256KB)
  - 這個位址非常接近 libc.so.6 等動態連結函式庫的區域。這是在 Memory Mapping Segment（記憶體映射區）。
  - 當申請的記憶體超過一個門檻（通常是 128 KB）時，malloc 會改用 mmap() 系統呼叫，直接在堆疊與堆積之間的空地「開闢」一塊獨立區域。
pmap 輸出解析
- 前五行 malloc 執行檔: 這是 malloc 程式本身，分為唯讀(程式碼本體)跟可讀寫(global variable)
- `[anon] 132K`: 這是 p1 所在的 heap，`anon` 代表 anonymous memory (匿名記憶體)，意思是他不對應硬碟上的任何檔案
- `libc.so.6` & `ld-linux...`：這是標準 C 函式庫與動態連結器。所有 C 程式都會載入它們。
- `[anon] 272K`: 這就是 p2 所在地。雖然你只要 256K (64×4096=262144 bytes)，但系統分配以 Page (4K) 為單位，加上一些管理資訊，最終呈現為 272K。
- `[stack]`:存放區域變數、函數呼叫記錄的地方。位址通常很高（以 0x7ffc 開頭）
- `vsyscall (ffffffffff600000)`：這是最頂層的位址，用於加速某些系統呼叫（如 `gettimeofday`）。

> 在 Stack 上：配置了一個指標變數 p（佔用 8 bytes，因為是 64 位元位址）。
在 Heap 上：配置了一塊 64 bytes 的匿名空間。
互動：p 裡面存的值，就是那塊 Heap 空間的「門牌號碼」（位址）。
Stack 是從高位址往低位址生長，Heap 是從低位址往高位址生長。它們中間隔著一段很大的空地（也就是你之前看到的 `[anon]` 區域）。這樣設計是為了最大化利用空間——誰需要多一點，就往中間長一點。
### 補充: linux OS memory management
可以從四個機制來理解: virtual memory, paging, demand paging & page fault, swap, kernel & user space
- virtual memory
  - Linux 不會讓程式直接存取實體 RAM 的位址。相反地，每個行程都有自己的虛擬位址空間（Virtual Address Space）。
  - Isolation：行程 A 無法讀取行程 B 的記憶體，這保證了系統的安全與穩定。
  - 超額分配 (Overcommit)：虛擬記憶體可以比實體記憶體大。例如你的 RAM 只有 16GB，但你可以執行多個總需求超過 16GB 的程式。
  透過幾個機制來達成：
    - lazy allocation
      - `malloc` 或是 `mmap` 配置 10 GB 記憶體的時候，不會真跟 RAM 要實體記憶體空間
      - kernel 只會在 process 的 virtual memory 畫出一塊 10GB 的記憶體，並回傳給 process
    - page fault
      - 當 process 要寫入某段記憶體的時候，CPU CR3 暫存器惠存 page table 的實體位址，呼叫 MMU 去找 page table，MMU 會拿 VPN (virtual page number) 去查 page table (存在於實體 RAM 上面) 有沒有對應的 page table entry。如果沒有 MMU 就會觸發 Exception (trap)，CPU 會將當前的 PC, register 等 infromation 壓入到 kernel stack。CPU 會把引起故障的 virtual memory 存在特定的暫存器 (x86 中是 CR2 暫存器)。之後 CPU 根據接收到的 interrupt vector number 去查 interrupt vector table(VT) 或是 IDT (interrupt descriptor table, x86)。每個 entry 為 16 bytes 的結構 (offset + selector)。這邊不多說 IDT entry 的內容。他們會透過 offset (透過 kernel page table，CPU 有暫存器指向 TSS 位址，TSS 會提供 kernel stack rsp) 去定位程式。
      - 補充： selector 會指向 GDT 提供 sementation descriptor (GDT 是 array，定義了三種段，code, data, system segment (TSS))。
    - COW, copy on write
