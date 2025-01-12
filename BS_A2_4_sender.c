#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <time.h>
#include <math.h>

#define NUM_ITERATIONS 10000
#define Z_VALUE 1.96 // für 95% Konfidenzintervall

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
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_PAIR);
    zmq_bind(socket, "tcp://*:5555");

    struct timespec start, end;
    double times[NUM_ITERATIONS];

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        zmq_send(socket, "Hello", 5, 0);
        zmq_recv(socket, NULL, 0, 0);
        clock_gettime(CLOCK_MONOTONIC, &end);

        // Änderung: Berechnung der verstrichenen Zeit in Nanosekunden
        times[i] = (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
    }

    double mean = calculate_mean(times, NUM_ITERATIONS);
    double std_dev = calculate_std(times, NUM_ITERATIONS, mean);
    double confidence_interval = calculate_confidence_interval(std_dev, NUM_ITERATIONS);

    // Änderung: Ausgabe der Zeit in Nanosekunden
    printf("Sender - Mittelwert der Zeit: %f Nanosekunden\n", mean);
    printf("Sender - Standardabweichung der Zeit: %f Nanosekunden\n", std_dev);
    printf("Sender - 95%% Konfidenzintervall: [%f, %f] Nanosekunden\n", mean - confidence_interval, mean + confidence_interval);

    zmq_close(socket);
    zmq_ctx_destroy(context);
    return 0;
}
