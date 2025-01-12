#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#define NUM_REQUESTS 100000
#define FILENAME "latencies.txt" // Name der Datei zum Speichern der Latenzen

int main (void) {
    printf("Connecting to hello world server…\n");
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, "tcp://localhost:5555");

    long latencies[NUM_REQUESTS];
    FILE *file = fopen(FILENAME, "w");
    if (file == NULL) {
        perror("Fehler beim Öffnen der Datei");
        exit(1);
    }

    for (int request_nbr = 0; request_nbr < NUM_REQUESTS; request_nbr++) {
        char buffer[20];
        struct timespec start, server_time;
        clock_gettime(CLOCK_MONOTONIC, &start);

        zmq_send(requester, (const char*)&start, sizeof(start), 0);
        zmq_recv(requester, buffer, sizeof(buffer), 0);

        memcpy(&server_time, buffer, sizeof(server_time));

        long nanoseconds = (server_time.tv_sec - start.tv_sec) * 1e9 + (server_time.tv_nsec - start.tv_nsec);
        latencies[request_nbr] = nanoseconds;
        fprintf(file, "%ld\n", nanoseconds);
    }

    fclose(file);
    zmq_close(requester);
    zmq_ctx_destroy(context);

    printf("Latenzen wurden in die Datei %s geschrieben\n", FILENAME);

    return 0;
}
