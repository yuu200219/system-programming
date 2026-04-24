// common.h
#include <pthread.h>

struct shared_data {
    pthread_mutex_t mutex;
    int count;
    char message[256];
};