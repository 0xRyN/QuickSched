#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sched.h"

#define DEBUG 0

typedef struct task {
    taskfunc func;
    void *arg;
    struct task *next;
} task;

typedef struct scheduler {
    task *stack;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int qlen;
    int count;
    int active_threads;
    bool active;
    pthread_t *threads;
} scheduler;

void *worker_thread(void *arg) {
    scheduler *s = (scheduler *)arg;
    task *t;

    while (true) {
        pthread_mutex_lock(&s->lock);
        while (!s->stack && s->active) {
            if (DEBUG)
                printf("Thread %lu sleeping\n", (unsigned long)pthread_self());
            pthread_cond_wait(&s->cond, &s->lock);
        }

        // If the scheduler is inactive and the stack is empty, exit the thread
        if (!s->active && !s->stack) {
            pthread_mutex_unlock(&s->lock);
            if (DEBUG)
                printf("Thread %lu exiting\n", (unsigned long)pthread_self());
            pthread_exit(NULL);
        }

        t = s->stack;
        s->stack = t->next;
        s->count--;

        // A new thread is running a task
        s->active_threads++;

        pthread_mutex_unlock(&s->lock);

        t->func(t->arg, s);
        free(t);

        // The thread has finished, decrement the active threads
        pthread_mutex_lock(&s->lock);
        s->active_threads--;

        // If it's the last thread and the stack is empty, disable the scheduler
        if (s->active_threads == 0 && !s->stack) {
            s->active = false;
            // Wake up all threads to exit
            pthread_cond_broadcast(&s->cond);
        }
        pthread_mutex_unlock(&s->lock);
    }
    return NULL;
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {
    if (nthreads == 0) {
        nthreads = sched_default_threads();
        printf("Using %i threads\n", nthreads);
    }

    scheduler *s = malloc(sizeof(scheduler));
    if (!s) return -1;

    s->stack = NULL;
    s->qlen = qlen;
    s->count = 0;
    s->active = true;
    pthread_mutex_init(&s->lock, NULL);
    pthread_cond_init(&s->cond, NULL);

    s->threads = malloc(sizeof(pthread_t) * nthreads);
    if (!s->threads) {
        free(s);
        return -1;
    }
    for (int i = 0; i < nthreads; i++) {
        pthread_create(&s->threads[i], NULL, worker_thread, s);
    }

    sched_spawn(f, closure, s);

    for (int i = 0; i < nthreads; i++) {
        pthread_join(s->threads[i], NULL);
    }

    pthread_mutex_destroy(&s->lock);
    pthread_cond_destroy(&s->cond);
    free(s->threads);
    free(s);

    return 1;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s) {
    task *new_task = malloc(sizeof(task));
    if (!new_task) return -1;

    new_task->func = f;
    new_task->arg = closure;
    new_task->next = NULL;

    pthread_mutex_lock(&s->lock);
    if (s->count >= s->qlen) {
        pthread_mutex_unlock(&s->lock);
        free(new_task);
        errno = EAGAIN;
        return -1;
    }

    new_task->next = s->stack;
    s->stack = new_task;
    s->count++;
    pthread_cond_signal(&s->cond);
    pthread_mutex_unlock(&s->lock);

    return 0;
}
