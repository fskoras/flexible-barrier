#include "flexible_barrier.h"

#include <unistd.h>
#include <pthread.h>

addr_flexbarrier_t barrier;

void* worker(void* arg) {
    size_t id = (size_t)arg;

    while (1) {
        printf("Thread %d: entering barrier\n", id);
        barrier_wait_result_t r = addr_flexbarrier_wait(&barrier);

        if (r == BARRIER_WAIT_EARLY) {
            printf("Thread %d: woke EARLY for extra work\n", id);
            sleep(1);
            printf("Thread %d: re-entering barrier\n", id);
            continue;
        }
        if (r == BARRIER_WAIT_CANCELLED) {
            printf("Thread %d: barrier CANCELLED, exiting...\n", id);
            break;
        }

        printf("Thread %d: barrier complete\n", id);
        sleep(1);
    }

    return NULL;
}

int main() {
    pthread_t t1, t2, t3;
    addr_flexbarrier_init(&barrier, 3);

    pthread_create(&t1, NULL, worker, (void*)1);
    pthread_create(&t2, NULL, worker, (void*)2);
    pthread_create(&t3, NULL, worker, (void*)3);

    sleep(2);
    printf("Main: cancelling barrier!\n");
    addr_flexbarrier_cancel(&barrier);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    addr_flexbarrier_destroy(&barrier);
    return 0;
}
