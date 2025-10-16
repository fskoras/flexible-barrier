#include "flexible_barrier.h"

void addr_flexbarrier_init(addr_flexbarrier_t *b, int threshold) {
    pthread_mutex_init(&b->mutex, NULL);
    pthread_cond_init(&b->cond, NULL);
    b->threads = calloc(threshold, sizeof(thread_entry_t));
    b->threshold = threshold;
    b->num_threads = 0;
    b->waiting = 0;
    b->generation = 0;
    b->cancelled = false;
}

void addr_flexbarrier_destroy(addr_flexbarrier_t *b) {
    free(b->threads);
    pthread_mutex_destroy(&b->mutex);
    pthread_cond_destroy(&b->cond);
}

static thread_entry_t *find_thread(addr_flexbarrier_t *b, pthread_t id) {
    for (int i = 0; i < b->num_threads; i++)
        if (pthread_equal(b->threads[i].id, id))
            return &b->threads[i];
    return NULL;
}

static thread_entry_t *register_thread(addr_flexbarrier_t *b, pthread_t id) {
    thread_entry_t *t = find_thread(b, id);
    if (t) return t;
    if (b->num_threads >= b->threshold) return NULL; // group full
    t = &b->threads[b->num_threads++];
    t->id = id;
    t->state = THREAD_ACTIVE;
    return t;
}

barrier_wait_result_t addr_flexbarrier_wait(addr_flexbarrier_t *b) {
    pthread_t self = pthread_self();
    pthread_mutex_lock(&b->mutex);

    // If barrier is already cancelled, exit immediately
    if (b->cancelled) {
        pthread_mutex_unlock(&b->mutex);
        return BARRIER_WAIT_CANCELLED;
    }

    thread_entry_t *me = register_thread(b, self);
    if (!me) {
        pthread_mutex_unlock(&b->mutex);
        return BARRIER_WAIT_CANCELLED;
    }

    // Reset to waiting, even if this thread was previously TEMP_RELEASED
    int my_gen = b->generation;
    me->state = THREAD_WAITING;
    b->waiting++;

    // Normal barrier completion
    if (b->waiting == b->threshold) {
        b->generation++;
        b->waiting = 0;
        for (int i = 0; i < b->num_threads; i++)
            b->threads[i].state = THREAD_ACTIVE;
        pthread_cond_broadcast(&b->cond);
        pthread_mutex_unlock(&b->mutex);
        return BARRIER_WAIT_NORMAL;
    }

    // Wait until barrier trips, canceled, or woken early
    while (!b->cancelled &&
           my_gen == b->generation &&
           me->state == THREAD_WAITING)
        pthread_cond_wait(&b->cond, &b->mutex);

    barrier_wait_result_t result = BARRIER_WAIT_NORMAL;
    if (b->cancelled)
        result = BARRIER_WAIT_CANCELLED;
    else if (me->state == THREAD_TEMP_RELEASED)
        result = BARRIER_WAIT_EARLY;

    me->state = THREAD_ACTIVE;
    pthread_mutex_unlock(&b->mutex);
    return result;
}

bool addr_flexbarrier_wake(addr_flexbarrier_t *b, pthread_t tid) {
    pthread_mutex_lock(&b->mutex);
    thread_entry_t *e = find_thread(b, tid);
    bool success = false;
    if (e && e->state == THREAD_WAITING) {
        e->state = THREAD_TEMP_RELEASED;
        b->waiting--;
        pthread_cond_broadcast(&b->cond); // wake that one
        success = true;
    }
    pthread_mutex_unlock(&b->mutex);
    return success;
}

void addr_flexbarrier_cancel(addr_flexbarrier_t *b) {
    pthread_mutex_lock(&b->mutex);
    b->cancelled = true;
    pthread_cond_broadcast(&b->cond); // wake everyone
    pthread_mutex_unlock(&b->mutex);
}

void addr_flexbarrier_reset(addr_flexbarrier_t *b) {
    pthread_mutex_lock(&b->mutex);
    b->cancelled = false;
    b->waiting = 0;
    b->generation++; // increment generation to mark a new phase
    for (int i = 0; i < b->num_threads; i++)
        b->threads[i].state = THREAD_ACTIVE;
    pthread_mutex_unlock(&b->mutex);
}
