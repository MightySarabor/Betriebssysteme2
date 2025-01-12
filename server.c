#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

int main (void)
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");
    assert (rc == 0);

    while (1) {
        struct timespec recv_time;
        zmq_recv (responder, &recv_time, sizeof(recv_time), 0);
        clock_gettime(CLOCK_MONOTONIC, &recv_time);

        zmq_send (responder, (const char*)&recv_time, sizeof(recv_time), 0);
    }
    return 0;
}
