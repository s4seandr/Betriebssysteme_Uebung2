#include <stdio.h>
#include <stdlib.h>
#include <zmq.h>
#include <time.h>

#define NUM_ITERATIONS 10000

int main() {
    void *context = zmq_ctx_new();
    void *socket = zmq_socket(context, ZMQ_PAIR);
    zmq_connect(socket, "tcp://localhost:5555");

    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        zmq_recv(socket, NULL, 0, 0);
        zmq_send(socket, "World", 5, 0);
    }

    zmq_close(socket);
    zmq_ctx_destroy(context);
    return 0;
}
