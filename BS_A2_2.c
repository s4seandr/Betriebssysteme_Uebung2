#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>

#define NUM_THREADS 2
#define NUM_ITERATIONS 10000
#define Z_VALUE 1.96 // für 95% Konfidenzintervall

sem_t semaphore;
int shared_data = 0;

// Funktionsprototypen
double calculate_mean(double* times, int size);
double calculate_std(double* times, int size, double mean);
double calculate_confidence_interval(double std_dev, int size);

void* thread_function(void* arg) {
    int id = *(int*)arg;
    struct timespec start, end;
    double elapsed;
    double times[NUM_ITERATIONS]; // Array zur Speicherung der Zeiten

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        sem_wait(&semaphore);
        shared_data++;
        sem_post(&semaphore);
        clock_gettime(CLOCK_MONOTONIC, &end);

        // Änderung: Berechnung der verstrichenen Zeit in Nanosekunden
        times[i] = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    }

    // Berechnung des Mittelwerts der gemessenen Zeiten in Nanosekunden
    double mean = calculate_mean(times, NUM_ITERATIONS);
    double std_dev = calculate_std(times, NUM_ITERATIONS, mean);
    double confidence_interval = calculate_confidence_interval(std_dev, NUM_ITERATIONS);

    // Änderung: Ausgabe der Zeit in Nanosekunden
    printf("Thread %d - Mittelwert der Zeit: %f Nanosekunden\n", id, mean);
    printf("Thread %d - Standardabweichung der Zeit: %f Nanosekunden\n", id, std_dev);
    printf("Thread %d - 95%% Konfidenzintervall: [%f, %f] Nanosekunden\n", id, mean - confidence_interval, mean + confidence_interval);

    pthread_exit(NULL);
}

double calculate_mean(double* times, int size) {
    double sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += times[i];
    }
    return sum / size;
}

double calculate_std(double* times, int size, double mean) {
    double sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += (times[i] - mean) * (times[i] - mean);
    }
    return sqrt(sum / size);
}

double calculate_confidence_interval(double std_dev, int size) {
    return Z_VALUE * (std_dev / sqrt(size));
}

int main() {
    pthread_t threads[NUM_THREADS];
    double times[NUM_THREADS];
    int ids[NUM_THREADS];

    sem_init(&semaphore, 0, 1);

    for (int i = 0; i < NUM_THREADS; ++i) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, thread_function, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    double mean = calculate_mean(times, NUM_THREADS);
    double std_dev = calculate_std(times, NUM_THREADS, mean);
    double confidence_interval = calculate_confidence_interval(std_dev, NUM_THREADS);

    printf("Final shared data: %d\n", shared_data);
    printf("Mittelwert der Zeit: %f Nanosekunden\n", mean);
    printf("Standardabweichung der Zeit: %f Nanosekunden\n", std_dev);
    printf("95%% Konfidenzintervall: [%f, %f] Nanosekunden\n", mean - confidence_interval, mean + confidence_interval);

    sem_destroy(&semaphore);
    return 0;
}
