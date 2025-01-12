#include <zmq.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define NUM_MESSAGES 100000
#define FILENAME "latencies.txt" // Name der Datei zum Speichern der Latenzen

long latencies[NUM_MESSAGES];

void* server_thread(void* context) {
    void* responder = zmq_socket(context, ZMQ_REP);
    zmq_bind(responder, "inproc://example");

    for (int i = 0; i < NUM_MESSAGES; i++) {
        char buffer[10];
        zmq_recv(responder, buffer, 10, 0);
        zmq_send(responder, "world", 5, 0);
    }

    zmq_close(responder);
    return NULL;
}

void* client_thread(void* context) {
    struct timespec start, end;
    void* requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, "inproc://example");

    FILE *file = fopen(FILENAME, "w");
    if (file == NULL) {
        perror("Fehler beim Öffnen der Datei");
        exit(1);
    }

    for (int i = 0; i < NUM_MESSAGES; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);

        zmq_send(requester, "Hallo", 5, 0);

        char buffer[10];
        zmq_recv(requester, buffer, 10, 0);

        clock_gettime(CLOCK_MONOTONIC, &end);

        long seconds = end.tv_sec - start.tv_sec;
        long nanoseconds = end.tv_nsec - start.tv_nsec;
        latencies[i] = seconds * 1000000000L + nanoseconds;
        fprintf(file, "%ld\n", latencies[i]);
    }

    fclose(file);
    zmq_close(requester);
    return NULL;
}

int main() {
    void* context = zmq_ctx_new();

    pthread_t server, client;
    pthread_create(&server, NULL, server_thread, context);
    sleep(1);  // Sicherstellen, dass der Server bereit ist
    pthread_create(&client, NULL, client_thread, context);

    pthread_join(client, NULL);
    pthread_join(server, NULL);

    zmq_ctx_destroy(context);
    printf("Latenzen wurden in die Datei %s geschrieben\n", FILENAME);

    return 0;
}  
