#include <zmq.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

long long current_timestamp_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long) ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int main() {
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    zmq_bind(responder, "tcp://*:5555");

    char buffer[10];
    for (int i = 0; i < 100000; i++) {
        zmq_recv(responder, buffer, 10, 0);

        long long server_timestamp = current_timestamp_ns();
        zmq_send(responder, &server_timestamp, sizeof(server_timestamp), 0);
    }

    zmq_close(responder);
    zmq_ctx_destroy(context);
    return 0;
}