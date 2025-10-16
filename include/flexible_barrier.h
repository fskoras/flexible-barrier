#ifndef FLEXIBLE_BARRIER_H
#define FLEXIBLE_BARRIER_H

#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {
    BARRIER_WAIT_NORMAL = 0,   // released when all threads reached
    BARRIER_WAIT_EARLY = 1,    // explicitly awakened by another thread
    BARRIER_WAIT_CANCELLED = 2 // optional for future use
} barrier_wait_result_t;

typedef enum {
    THREAD_ACTIVE,
    THREAD_WAITING,
    THREAD_TEMP_RELEASED
} thread_state_t;

typedef struct {
    pthread_t id;
    thread_state_t state;
} thread_entry_t;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    thread_entry_t *threads;
    int threshold;      // total threads that must arrive
    int num_threads;    // registered threads
    int waiting;        // number currently waiting
    int generation;     // barrier "round" counter
    bool cancelled;     // barrier canceled flag
} addr_flexbarrier_t;

void addr_flexbarrier_init(addr_flexbarrier_t *b, int threshold);
void addr_flexbarrier_destroy(addr_flexbarrier_t *b);
barrier_wait_result_t addr_flexbarrier_wait(addr_flexbarrier_t *b);
bool addr_flexbarrier_wake(addr_flexbarrier_t *b, pthread_t tid);
void addr_flexbarrier_cancel(addr_flexbarrier_t *b);
void addr_flexbarrier_reset(addr_flexbarrier_t *b);

#endif //!FLEXIBLE_BARRIER
