#include "tpool.h"
#include <unistd.h>

static void* routine_thread_func(void* arg) {
    struct tpool* pool = (struct tpool*) arg;
    struct tpool_work* work = NULL;
    while (1) {
        pthread_mutex_lock(&pool->queue_mutex);
        while (pool->queue_head == NULL && !pool->shutdown) {
            while(pthread_cond_wait(&pool->queue_ready, &pool->queue_mutex) != 0);
#ifdef DEBUG
            printf("wait over");
#endif
        }
        if (pool->shutdown) {
            pthread_mutex_unlock(&pool->queue_mutex);
            pthread_exit(NULL);
        }
        work = pool->queue_head;
        pool->queue_head = work->next;
        pthread_mutex_unlock(&pool->queue_mutex);
        work->func(work->arg);
        free(work);
    }
    return NULL;
}

int tpool_create(struct tpool* pool, int thread_num) {
    pool->thread_handles = malloc(thread_num * sizeof(pthread_t));
    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_ready, NULL);
    pool->shutdown = 0;
    pool->thread_num = thread_num;
    pool->queue_head = NULL;
    for (int i = 0; i < thread_num; ++i) {
        pthread_create(&pool->thread_handles[i], NULL, routine_thread_func, (void*)pool);
    }
    return 0;
}

int tpool_wait(struct tpool* pool) {
    while (pool->queue_head != NULL);
    return 0;
}

int tpool_destroy(struct tpool* pool) {
    if (pool->shutdown) {
        return 0;
    }
    int thread_num = pool->thread_num;
    struct tpool_work* tmp_work = NULL;
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->queue_ready);
    pthread_mutex_unlock(&pool->queue_mutex);
    for (int i = 0; i < thread_num; ++i) {
        pthread_join(pool->thread_handles[i], NULL);
    }
    while (pool->queue_head != NULL) {
        tmp_work = pool->queue_head;
        pool->queue_head = tmp_work->next;
        free(tmp_work);
    }
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_ready);
    free(pool->thread_handles);
    return 0;
}

int tpool_add_task(struct tpool* pool, void* (*task_func) (void*), void* arg) {
    struct tpool_work* work = malloc(sizeof(struct tpool_work));
    work->func = task_func;
    work->arg = arg;
    work->next = NULL;
    pthread_mutex_lock(&pool->queue_mutex);
    if (pool->queue_head == NULL) {
        pool->queue_head = work;
    }
    else {
        struct tpool_work* work_end = pool->queue_head;
        while (work_end->next != NULL) {
            work_end = work_end->next;
        }
        work_end->next = work;
    }
    pthread_cond_signal(&pool->queue_ready);
    pthread_mutex_unlock(&pool->queue_mutex);
#ifdef DEBUG
    printf("add\n");
#endif
    return 0;
}
