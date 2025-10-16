#include "flexible_barrier.h"

#include <unistd.h>
#include <pthread.h>

addr_flexbarrier_t barrier;

void* worker(void* arg) {
    size_t id = (size_t)arg;
    printf("Thread %d: waiting at barrier...\n", id);

    barrier_wait_result_t r = addr_flexbarrier_wait(&barrier);
    if (r == BARRIER_WAIT_EARLY) {
        printf("Thread %d: woke EARLY (doing extra work)\n", id);
        sleep(1);
        printf("Thread %d: done extra work, re-waiting\n", id);
        addr_flexbarrier_wait(&barrier);
    } else if (r == BARRIER_WAIT_NORMAL) {
        printf("Thread %d: woke NORMALLY\n", id);
    } else {
        printf("Thread %d: barrier CANCELLED or ERROR\n", id);
    }

    printf("Thread %d: passed barrier\n", id);
    return NULL;
}

int main() {
    pthread_t t1, t2, t3;
    addr_flexbarrier_init(&barrier, 3);

    pthread_create(&t1, NULL, worker, (void*)1);
    pthread_create(&t2, NULL, worker, (void*)2);

    sleep(1);
    printf("Main: explicitly waking thread 1\n");
    addr_flexbarrier_wake(&barrier, t1);

    sleep(1);
    pthread_create(&t3, NULL, worker, (void*)3);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    addr_flexbarrier_destroy(&barrier);
    return 0;
}
