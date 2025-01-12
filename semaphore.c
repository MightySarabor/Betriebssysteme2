#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define NUM_EXPERIMENTS 100000 // Anzahl der Experimente
#define FILENAME "latencies.txt" // Name der Datei zum Speichern der Latenzen

typedef struct {
    struct timespec *start_times;
    struct timespec *end_times;
    int experiment_index;
    pthread_mutex_t *mutex;
} thread_data_t;

void *thread_function(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;

    pthread_mutex_lock(data->mutex); // Mutex sperren

    clock_gettime(CLOCK_MONOTONIC_RAW, &data->end_times[data->experiment_index]); // Zeit des Erwerbs des Mutex erfassen

    pthread_mutex_unlock(data->mutex); // Mutex freigeben
    return NULL;
}

void *release_thread_function(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;

    clock_gettime(CLOCK_MONOTONIC_RAW, &data->start_times[data->experiment_index]); // Zeit der Freigabe des Mutex erfassen
    pthread_mutex_unlock(data->mutex); // Mutex freigeben (auf 0 setzen)
    return NULL;
}

void calculate_latency(struct timespec *start_times, struct timespec *end_times, long long *latencies) {
    for (int i = 0; i < NUM_EXPERIMENTS; i++) {
        struct timespec latency;
        latency.tv_sec = end_times[i].tv_sec - start_times[i].tv_sec;
        latency.tv_nsec = end_times[i].tv_nsec - start_times[i].tv_nsec;
        if (latency.tv_nsec < 0) {
            latency.tv_sec--;
            latency.tv_nsec += 1000000000;
        }
        latencies[i] = latency.tv_sec * 1000000000 + latency.tv_nsec;
    }
}

int main() {
    pthread_t thread1, thread2;
    struct timespec start_times[NUM_EXPERIMENTS];
    struct timespec end_times[NUM_EXPERIMENTS];
    long long latencies[NUM_EXPERIMENTS];
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex initialisieren

    thread_data_t data;
    data.start_times = start_times;
    data.end_times = end_times;
    data.mutex = &mutex;

    for (int i = 0; i < NUM_EXPERIMENTS; i++) {
        data.experiment_index = i;
        pthread_mutex_lock(&mutex); // Mutex initial sperren

        pthread_create(&thread2, NULL, thread_function, (void *)&data);
        usleep(500); // Kurze Pause, um sicherzustellen, dass der wartende Thread zuerst läuft
        pthread_create(&thread1, NULL, release_thread_function, (void *)&data);

        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);
    }

    calculate_latency(start_times, end_times, latencies);

    // Latenzen in Datei schreiben
    FILE *file = fopen(FILENAME, "w");
    if (file == NULL) {
        perror("Fehler beim Öffnen der Datei");
        exit(1);
    }

    for (int i = 0; i < NUM_EXPERIMENTS; i++) {
        fprintf(file, "%lld\n", latencies[i]);
    }

    fclose(file);

    printf("Latenzen wurden in die Datei %s geschrieben\n", FILENAME);

    return 0;
}
