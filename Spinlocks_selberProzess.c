#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>

#define NUM_THREADS 2
#define NUM_ITERATIONS 10000

volatile int lock = 0;

void acquire_spinlock(volatile int *lock) {
    while (__sync_lock_test_and_set(lock, 1)) {
        // Busy-wait
    }
}

void release_spinlock(volatile int *lock) {
    __sync_lock_release(lock);
}

int compare(const void *a, const void *b) {
    return (*(long *)a - *(long *)b);
}

void *thread_func(void *arg) {
    int id = *((int *)arg);
    long elapsed_times[NUM_ITERATIONS];
    struct timespec start, end;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // Start measurement
        clock_gettime(CLOCK_MONOTONIC, &start);

        // Acquire lock
        acquire_spinlock(&lock);

        // Critical section (simulation)
        usleep(1000); // 1 millisecond

        // Release lock
        release_spinlock(&lock);

        // End measurement
        clock_gettime(CLOCK_MONOTONIC, &end);

        // Calculate elapsed time in nanoseconds
        elapsed_times[i] = (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);
    }

    // Calculate the minimum, average, and median elapsed times
    long min_time = LONG_MAX;
    long sum_time = 0;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        if (elapsed_times[i] < min_time) {
            min_time = elapsed_times[i];
        }
        sum_time += elapsed_times[i];
    }

    long avg_time = sum_time / NUM_ITERATIONS;

    qsort(elapsed_times, NUM_ITERATIONS, sizeof(long), compare);
    long median_time = (NUM_ITERATIONS % 2 == 0) ? (elapsed_times[NUM_ITERATIONS / 2 - 1] + elapsed_times[NUM_ITERATIONS / 2]) / 2 : elapsed_times[NUM_ITERATIONS / 2];

    // Print the results
    printf("Thread %d: Minimum elapsed time = %ld ns\n", id, min_time);
    printf("Thread %d: Average elapsed time = %ld ns\n", id, avg_time);
    printf("Thread %d: Median elapsed time = %ld ns\n", id, median_time);

    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_func, &thread_ids[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}