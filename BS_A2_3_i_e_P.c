#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <zmq.h>
#include <time.h>
#include <math.h>

#define NUM_THREADS 2
#define NUM_ITERATIONS 10000
#define Z_VALUE 1.96 // für 95% Konfidenzintervall

typedef struct {
    int id;
    void *context;
    void *socket;
} thread_data;

// Funktionsprototypen
double calculate_mean(double* times, int size);
double calculate_std(double* times, int size, double mean);
double calculate_confidence_interval(double std_dev, int size);

void* thread_function(void* arg) {
    thread_data *data = (thread_data*)arg;
    struct timespec start, end;
    double elapsed;
    double times[NUM_ITERATIONS]; // Array zur Speicherung der Zeiten

    // ZeroMQ Kontext und Socket initialisieren
    void *context = data->context;
    void *socket = zmq_socket(context, ZMQ_PAIR);
    if (data->id == 0) {
        printf("Thread %d binding...\n", data->id);
        zmq_bind(socket, "inproc://test");
    } else {
        printf("Thread %d connecting...\n", data->id);
        zmq_connect(socket, "inproc://test");
    }

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        if (data->id == 0) {
            zmq_send(socket, "Hello", 5, 0);
            zmq_recv(socket, NULL, 0, 0);
        } else {
            zmq_recv(socket, NULL, 0, 0);
            zmq_send(socket, "World", 5, 0);
        }
        if (i % 1000 == 0) {
            printf("Thread %d: Iteration %d\n", data->id, i);
        }
        clock_gettime(CLOCK_MONOTONIC, &end);

        // Änderung: Berechnung der verstrichenen Zeit in Nanosekunden
        times[i] = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    }

    // Berechnung des Mittelwerts der gemessenen Zeiten in Nanosekunden
    double mean = calculate_mean(times, NUM_ITERATIONS);
    double std_dev = calculate_std(times, NUM_ITERATIONS, mean);
    double confidence_interval = calculate_confidence_interval(std_dev, NUM_ITERATIONS);

    // Änderung: Ausgabe der Zeit in Nanosekunden
    printf("Thread %d - Mittelwert der Zeit: %f Nanosekunden\n", data->id, mean);
    printf("Thread %d - Standardabweichung der Zeit: %f Nanosekunden\n", data->id, std_dev);
    printf("Thread %d - 95%% Konfidenzintervall: [%f, %f] Nanosekunden\n", data->id, mean - confidence_interval, mean + confidence_interval);

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
    thread_data data[NUM_THREADS];
    void *context = zmq_ctx_new();

    for (int i = 0; i < NUM_THREADS; ++i) {
        data[i].id = i;
        data[i].context = context;
        pthread_create(&threads[i], NULL, thread_function, &data[i]);
    }

    for (int i = 0; i < NUM_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }

    zmq_ctx_destroy(context);
    return 0;
}
