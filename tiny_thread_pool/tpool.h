#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

struct tpool_work{
    void* arg;
    void* (*func) (void*);
    struct tpool_work* next;
};

struct tpool {
    pthread_t* thread_handles;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_ready;

    int shutdown;
    size_t thread_num;
    struct tpool_work* queue_head;
};

int tpool_create(struct tpool* pool, int thread_num);

int tpool_wait(struct tpool* pool);

int tpool_destroy(struct tpool* pool);

int tpool_add_task(struct tpool* pool, void* (*task_func) (void*), void* arg);
