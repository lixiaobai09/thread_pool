#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_QUEUE_SIZE (1024)
struct tpool_work {
    void* arg;
    void* (*func) (void*);
};

struct tpool {
    pthread_t* thread_handles;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_ready;
    pthread_cond_t queue_empty;
    pthread_cond_t queue_space; // does queue have space to push task?

    int shutdown;   // 0 is don't shutdown, 1 is yes
    size_t thread_num; // the number of threads in pool
    size_t task_cnt;   // count thread pool tasks
    size_t task_limit;   // the limit number of tasks in pool
    struct tpool_work* task_queue;
    int queue_head, queue_end;
};

int tpool_create_with_task_limit(struct tpool* tpool, int thread_num, int max_task_num); // create the threads pool
int tpool_create(struct tpool* tpool, int thread_num); // create the threads pool

int tpool_destroy(struct tpool* tpool); // destroy the threads pool

int tpool_wait(struct tpool* tpool);  // wait all tasks in threads pool run over

/**
 * add a task to threads pool
 * @ immed  return the result immediately
 * if task queue is full and immed = 1, return value 1;
 * if task queue is full and immed = 0, just wait for queue, and insert the task
 *
**/
int tpool_add_task(struct tpool* tpool, int immed, void* (*task_func) (void*), void* arg); // add a task to threads pool
