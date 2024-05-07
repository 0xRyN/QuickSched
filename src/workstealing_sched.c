#if defined(__linux__)
#define _POSIX_C_SOURCE 199309L
#define _DEFAULT_SOURCE
#endif

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "sched.h"

#define BASE_WAIT_MICROSECS 50
#define BACKOFF_FACTOR 2.0
#define MAX_WAIT_MICROSECS 1000000

void wait_exponential_backoff(int attempt) {
    if (attempt > 0) {
        int wait_time =
            (int)(BASE_WAIT_MICROSECS * pow(BACKOFF_FACTOR, attempt - 1));
        if (wait_time > MAX_WAIT_MICROSECS) {
            wait_time = MAX_WAIT_MICROSECS;
        }
        usleep(wait_time);
    }
}

#define DEBUG 0

typedef struct task {
    taskfunc func;
    void *arg;
    struct task *prev;
    struct task *next;
} task;

typedef struct deque {
    task *bottom;
    task *top;
    pthread_mutex_t lock;
} deque;

typedef struct scheduler {
    int nthreads;
    deque *deques;
    pthread_t *threads;
    atomic_bool *active_threads;
    atomic_bool active;
} scheduler;

int find_thread_id(scheduler *s, pthread_t thread_id) {
    for (int i = 0; i < s->nthreads; i++) {
        if (pthread_equal(s->threads[i], thread_id)) {
            return i;
        }
    }
    printf("Thread not found\n");
    return -1;
}

int tasks_by_thread[100];

// No need for synchronization here, as each thread will only access its own
void log_task(int thread_id, scheduler *s) { tasks_by_thread[thread_id]++; }

pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

int deque_number_of_tasks(deque *dq) {
    int count = 0;
    task *t = dq->bottom;
    while (t) {
        count++;
        t = t->next;
    }
    return count;
}

void print_deque_states(int thread_id, scheduler *s) {
    if (!DEBUG) return;
    pthread_mutex_lock(&print_lock);
    printf("=== THREAD %d === [", thread_id);
    for (int i = 0; i < s->nthreads; i++) {
        printf("%d", deque_number_of_tasks(&s->deques[i]));
        if (i < s->nthreads - 1) {
            printf(", ");
        }
    }
    printf("]\n");
    pthread_mutex_unlock(&print_lock);
}

void push_bottom(deque *dq, task *t) {
    pthread_mutex_lock(&dq->lock);
    if (dq->bottom == NULL) {
        dq->top = dq->bottom = t;
        t->prev = t->next = NULL;
    } else {
        t->next = dq->bottom;
        dq->bottom->prev = t;
        t->prev = NULL;
        dq->bottom = t;
    }
    pthread_mutex_unlock(&dq->lock);
}

task *pop_bottom(deque *dq) {
    pthread_mutex_lock(&dq->lock);
    task *t = dq->bottom;
    if (t == NULL) {
        pthread_mutex_unlock(&dq->lock);
        return NULL;
    }
    if (dq->bottom == dq->top) {
        dq->bottom = dq->top = NULL;
    } else {
        dq->bottom = t->next;
        dq->bottom->prev = NULL;
    }
    pthread_mutex_unlock(&dq->lock);
    return t;
}

task *pop_top(deque *dq) {
    pthread_mutex_lock(&dq->lock);
    task *t = dq->top;
    if (t == NULL) {
        pthread_mutex_unlock(&dq->lock);
        return NULL;
    }
    if (dq->top == dq->bottom) {
        dq->top = dq->bottom = NULL;
    } else {
        dq->top = t->prev;
        dq->top->next = NULL;
    }
    pthread_mutex_unlock(&dq->lock);
    return t;
}

task *try_steal_task(scheduler *s, int my_id) {
    int n = s->nthreads;
    int start = rand() % n;
    for (int i = 0; i < n; i++) {
        int idx = (start + i) % n;
        if (idx == my_id) continue;
        task *t = pop_top(&s->deques[idx]);
        if (t) {
            return t;
        }
    }
    return NULL;
}

void check_active_threads(scheduler *s) {
    for (int i = 0; i < s->nthreads; i++) {
        if (atomic_load(&s->active_threads[i])) return;
    }

    atomic_store(&s->active, false);
}

void activate_threads(scheduler *s) {
    for (int i = 0; i < s->nthreads; i++) {
        atomic_store(&s->active_threads[i], true);
    }
}

void *worker_thread(void *arg) {
    scheduler *s = (scheduler *)arg;
    int my_id = find_thread_id(s, pthread_self());
    deque *my_deque = &s->deques[my_id];
    task *t;
    int backoff_attempts = 0;

    atomic_store(&s->active_threads[my_id], true);
    while (atomic_load(&s->active)) {
        t = pop_bottom(my_deque);
        if (!t) {
            t = try_steal_task(s, my_id);
            if (!t) {
                atomic_store(&s->active_threads[my_id], false);
                check_active_threads(s);
                wait_exponential_backoff(backoff_attempts++);
                continue;
            }

            else {
                backoff_attempts = 0;
            }
        }

        else {
            backoff_attempts = 0;
        }

        if (!atomic_load(&s->active_threads[my_id])) {
            atomic_store(&s->active_threads[my_id], true);
        }

        if (DEBUG) log_task(my_id, s);
        t->func(t->arg, s);
        free(t);
    }
    if (DEBUG) printf("Thread %d - Peacing out\n", my_id);
    return NULL;
}

int sched_init(int nthreads, int qlen, taskfunc f, void *closure) {
    if (nthreads == 0) {
        nthreads = sched_default_threads();
        printf("Using %i threads\n", nthreads);
    }

    scheduler *s = malloc(sizeof(scheduler));
    if (!s) return -1;

    s->nthreads = nthreads;
    s->deques = malloc(sizeof(deque) * nthreads);
    if (!s->deques) {
        free(s);
        return -1;
    }

    s->threads = malloc(sizeof(pthread_t) * nthreads);
    if (!s->threads) {
        free(s->deques);
        free(s);
        return -1;
    }

    s->active_threads = malloc(sizeof(atomic_bool) * nthreads);
    if (!s->active_threads) {
        free(s->threads);
        free(s->deques);
        free(s);
        return -1;
    }

    // activate_threads(s);

    for (int i = 0; i < nthreads; i++) {
        s->deques[i].bottom = s->deques[i].top = NULL;
        pthread_mutex_init(&s->deques[i].lock, NULL);
    }

    atomic_store(&s->active, true);

    sched_spawn(f, closure, s);

    for (int i = 0; i < nthreads; i++) {
        pthread_create(&s->threads[i], NULL, worker_thread, s);
    }

    for (int i = 0; i < nthreads; i++) {
        pthread_join(s->threads[i], NULL);
    }

    free(s->deques);
    free(s->threads);
    free(s);

    return 0;
}

int sched_spawn(taskfunc f, void *closure, struct scheduler *s) {
    pthread_t self = pthread_self();
    int thread_id = -1;
    for (int i = 0; i < s->nthreads; i++) {
        if (pthread_equal(s->threads[i], self)) {
            thread_id = i;
            break;
        }
    }

    if (thread_id == -1) {
        if (DEBUG) printf("Called by main thread\n");
        thread_id = 0;
    }

    task *new_task = malloc(sizeof(task));
    if (!new_task) return -1;

    new_task->func = f;
    new_task->arg = closure;
    new_task->prev = NULL;
    new_task->next = NULL;

    push_bottom(&s->deques[thread_id], new_task);

    return 0;
}

/*
 * This version of the initializer returns a pointer to the scheduler
 * it makes it easy to add more tasks to the scheduler
 * --
 * This initializer won't stop when all threads are sleeping.
 * Use this with graphical applications
 */

scheduler *stealing_sched_init(int nthreads, int qlen, taskfunc f,
                               void *closure) {
    if (nthreads == 0) {
        nthreads = sched_default_threads();
        printf("Using %i threads\n", nthreads);
    }

    scheduler *s = malloc(sizeof(scheduler));
    if (!s) return NULL;

    s->nthreads = nthreads;
    s->deques = malloc(sizeof(deque) * nthreads);
    if (!s->deques) {
        free(s);
        return NULL;
    }

    s->threads = malloc(sizeof(pthread_t) * nthreads);
    if (!s->threads) {
        free(s->deques);
        free(s);
        return NULL;
    }

    s->active_threads = malloc(sizeof(atomic_bool) * nthreads);
    if (!s->active_threads) {
        free(s->threads);
        free(s->deques);
        free(s);
        return NULL;
    }

    for (int i = 0; i < nthreads; i++) {
        s->deques[i].bottom = s->deques[i].top = NULL;
        pthread_mutex_init(&s->deques[i].lock, NULL);
    }

    atomic_store(&s->active, true);

    sched_spawn(f, closure, s);

    for (int i = 0; i < nthreads; i++) {
        pthread_create(&s->threads[i], NULL, worker_thread, s);
    }

    return s;
}