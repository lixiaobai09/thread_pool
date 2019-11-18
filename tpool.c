#include "tpool.h"

static void* routine_thread_func(void* arg) {
    struct tpool* pool = (struct tpool*) arg;
    struct tpool_work* work = NULL;
    while(1) {
        pthread_mutex_lock(&pool->queue_mutex);
        while (!pool->task_cnt && !pool->shutdown) { // wait for task push into queue
            while (pthread_cond_wait(&pool->queue_ready, &pool->queue_mutex) != 0);
        }
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->queue_mutex);
            pthread_exit(NULL);
        }
        work = &pool->task_queue[pool->queue_head];
        pool->queue_head = (pool->queue_head + 1) % pool->task_limit;
        pthread_mutex_unlock(&pool->queue_mutex);
        // run task
        work->func(work->arg);
        pthread_mutex_lock(&pool->queue_mutex);
        if (pool->task_cnt == pool->task_limit) {
            pthread_cond_signal(&pool->queue_space);
        }
        -- pool->task_cnt;
        if (!pool->task_cnt) {
            pthread_cond_signal(&pool->queue_empty);
        }
        pthread_mutex_unlock(&pool->queue_mutex);
    }

    return NULL;
}

int tpool_create_with_task_limit(struct tpool* pool, int thread_num, int max_task_num) { // create the threads pool
    pool->thread_num = thread_num;
    pool->task_limit = max_task_num;
    pool->task_cnt = 0;
    pool->shutdown = 0;
    pool->queue_end = pool->queue_head = 0;
    pool->task_queue = malloc(max_task_num * sizeof(struct tpool_work));
    pool->thread_handles = malloc(thread_num * sizeof(pthread_t));
    if(pthread_mutex_init(&pool->queue_mutex, NULL) != 0) {
        perror("queue mutex init: ");
        return -1;
    }
    if (pthread_cond_init(&pool->queue_ready, NULL) != 0) {
        perror("queue ready condition: ");
        return -1;
    }
    if (pthread_cond_init(&pool->queue_empty, NULL) != 0) {
        perror("queue empty condition: ");
        return -1;
    }
    if (pthread_cond_init(&pool->queue_space, NULL) != 0) {
        perror("queue space condition: ");
        return -1;
    }
    pthread_t* thread_handles = pool->thread_handles;
    for (long i = 0; i < thread_num; ++i) {
        if (pthread_create(&thread_handles[i], NULL, routine_thread_func, (void*)pool) != 0) {
            perror("create thread for pool: ");
            return -1;
        }
    }
    return 0;
}

int tpool_create(struct tpool* pool, int thread_num) { // create the threads pool
    return tpool_create_with_task_limit(pool, thread_num, DEFAULT_QUEUE_SIZE);
}

int tpool_destroy(struct tpool* pool) { // direct destroy the threads pool
    size_t num;
    struct tpool_work* work = NULL;
    if (pool->shutdown) {
        return 0;
    }
    if (pthread_mutex_lock(&pool->queue_mutex) != 0) {
        perror("destroy: ");
        return -1;
    }
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->queue_ready);
    if (pthread_mutex_unlock(&pool->queue_mutex) != 0) {
        perror("destroy: ");
        return -1;
    }
    num = pool->thread_num;
    pthread_t* thread_handles = pool->thread_handles;
    for (long i = 0; i < num; ++i) {
        if (pthread_join(thread_handles[i], NULL) != 0) {
            perror("destroy thread: ");
            return -1;
        }
    }
    pool->queue_end = pool->queue_head = 0;
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_ready);
    pthread_cond_destroy(&pool->queue_empty);
    pthread_cond_destroy(&pool->queue_space);
    free(pool->task_queue);
    free(pool->thread_handles);
    pool->thread_handles = NULL;
    pool->task_queue = NULL;
    return 0;
}

int tpool_wait(struct tpool* pool) {  // wait all tasks in threads pool run over
    pthread_mutex_lock(&pool->queue_mutex);
    if (!pool->task_cnt) {
        while(pthread_cond_wait(&pool->queue_empty, &pool->queue_mutex) != 0);
    }
    pthread_mutex_unlock(&pool->queue_mutex);
    return 0;
}
/**
 * add a task to threads pool
 * @ immed  return the result immediately
 * if task queue is full and immed = 1, return value 1;
 * if task queue is full and immed = 0, just wait for queue, and insert the task
 *
**/
int tpool_add_task(struct tpool* pool, int immed, void* (*task_func) (void*), void* arg) {
    pthread_mutex_lock(&pool->queue_mutex);
    if (pool->task_cnt == pool->task_limit) {
        if (immed) {
            pthread_mutex_unlock(&pool->queue_mutex);
            return 1;
        }
        while (pthread_cond_wait(&pool->queue_space, &pool->queue_mutex) != 0);
    }
    struct tpool_work* task = (pool->task_queue + pool->queue_end);
    task->func = task_func;
    task->arg = arg;
    pool->queue_end = (pool->queue_end + 1) % pool->task_limit;
    ++ pool->task_cnt;
    pthread_cond_signal(&pool->queue_ready);
    pthread_mutex_unlock(&pool->queue_mutex);
    return 0;
}
