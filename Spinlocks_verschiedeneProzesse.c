#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <limits.h>

#define NUM_ITERATIONS 10000
#define SHM_NAME "/shm_spinlock"

struct shared_data {
    volatile int lock;
};

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

void measure_latency(struct shared_data *data) {
    long elapsed_times[NUM_ITERATIONS];
    struct timespec start, end;

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // Start measurement
        clock_gettime(CLOCK_MONOTONIC, &start);

        // Acquire lock
        acquire_spinlock(&(data->lock));

        // Critical section (simulation)
        usleep(1000); // 1 millisecond

        // Release lock
        release_spinlock(&(data->lock));

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
    printf("Minimum elapsed time = %ld ns\n", min_time);
    printf("Average elapsed time = %ld ns\n", avg_time);
    printf("Median elapsed time = %ld ns\n", median_time);
}

int main() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(struct shared_data));
    struct shared_data *data = mmap(0, sizeof(struct shared_data), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (fork() == 0) { // Child process
        measure_latency(data);
        munmap(data, sizeof(struct shared_data));
        close(shm_fd);
    } else { // Parent process
        measure_latency(data);
        munmap(data, sizeof(struct shared_data));
        close(shm_fd);
        shm_unlink(SHM_NAME);
    }

    return 0;
}
