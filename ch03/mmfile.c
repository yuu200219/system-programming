#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

const size_t k=1024;
const size_t m=k*k;
const size_t g=k*m;
const size_t t=k*g;

int main (int argc, char** argv) {
    const int fileSize = 500000;
    char* mmfile=NULL;
    char* stringBuf="end";
    int fd = open("./mmfile.data", O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
    // 1. 先跳到 500,000 - 3 的位置
    lseek(fd, fileSize-sizeof(stringBuf), SEEK_SET);
    // 2. 從那個位置開始寫入 3 bytes ("end")
    assert(write(fd, stringBuf, sizeof(stringBuf))==sizeof(stringBuf));
    //sync();
    //exit(0);
    mmfile = mmap(NULL, fileSize, PROT_WRITE, MAP_SHARED, fd, 0);
    assert(mmfile != NULL);
    for (int i=0; i< fileSize; i++) {
        mmfile[i] = 'B';
    }
}
