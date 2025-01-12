#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define NUM_MESSAGES 100000
#define NUM_BOOTSTRAPS 10000

long long current_timestamp_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (long long) ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int compare_longs_asc(const void *a, const void *b) {
    long long arg1 = *(const long long *)a;
    long long arg2 = *(const long long *)b;
    return (arg1 > arg2) - (arg1 < arg2);
}

int compare_longs_desc(const void *a, const void *b) {
    long long arg1 = *(const long long *)a;
    long long arg2 = *(const long long *)b;
    return (arg1 < arg2) - (arg1 > arg2);
}

double calculate_median(long long *latencies, int size) {
    qsort(latencies, size, sizeof(long long), compare_longs_asc);
    if (size % 2 == 0) {
        return (latencies[size / 2 - 1] + latencies[size / 2]) / 2.0;
    } else {
        return latencies[size / 2];
    }
}

void bootstrap_confidence_interval(long long *latencies, int size, int num_bootstraps, double *ci_lower, double *ci_upper) {
    double *bootstrap_medians = malloc(num_bootstraps * sizeof(double));
    if (!bootstrap_medians) {
        perror("Fehler beim Allozieren des Speichers");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_bootstraps; i++) {
        long long *bootstrap_sample = malloc(size * sizeof(long long));
        if (!bootstrap_sample) {
            perror("Fehler beim Allozieren des Speichers");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < size; j++) {
            int random_index = rand() % size;
            bootstrap_sample[j] = latencies[random_index];
        }

        bootstrap_medians[i] = calculate_median(bootstrap_sample, size);
        free(bootstrap_sample);
    }

    qsort(bootstrap_medians, num_bootstraps, sizeof(double), compare_longs_asc);
    *ci_lower = bootstrap_medians[(int)(num_bootstraps * 0.025)];
    *ci_upper = bootstrap_medians[(int)(num_bootstraps * 0.975)];

    free(bootstrap_medians);
}

int main() {
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, "tcp://server:5555");

    long long latencies[NUM_MESSAGES];
    int valid_latencies_count = 0;
    int negative_latencies_count = 0;

    for (int i = 0; i < NUM_MESSAGES; i++) {
        long long client_timestamp_send = current_timestamp_ns();
        zmq_send(requester, "Hello", 5, 0);

        long long server_timestamp_receive;
        zmq_recv(requester, &server_timestamp_receive, sizeof(server_timestamp_receive), 0);

        long long latency = server_timestamp_receive - client_timestamp_send;

        if (latency >= 0) {
            latencies[valid_latencies_count++] = latency;
        } else {
            negative_latencies_count++;
        }
    }

    zmq_close(requester);
    zmq_ctx_destroy(context);

    // Analyse der Latenzen
    qsort(latencies, valid_latencies_count, sizeof(long long), compare_longs_desc);
    printf("3 höchste Latenzen: %lld ns, %lld ns, %lld ns\n", latencies[0], latencies[1], latencies[2]);

    qsort(latencies, valid_latencies_count, sizeof(long long), compare_longs_asc);
    printf("3 niedrigste Latenzen: %lld ns, %lld ns, %lld ns\n", latencies[0], latencies[1], latencies[2]);

    double median = calculate_median(latencies, valid_latencies_count);
    double ci_lower, ci_upper;
    bootstrap_confidence_interval(latencies, valid_latencies_count, NUM_BOOTSTRAPS, &ci_lower, &ci_upper);

    printf("Median der Latenz: %.2f ns\n", median);
    printf("Konfidenzintervall (95%%): [%.2f ns, %.2f ns]\n", ci_lower, ci_upper);

    // Ausgabe der negativen Latenzen
    printf("Anzahl der negativen Latenzen: %d\n", negative_latencies_count);

    return 0;
}
