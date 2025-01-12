#include <stdio.h>
#include <stdlib.h>

#define FILENAME "latencies.txt"
#define NUM_EXPERIMENTS 10000
#define NUM_BOOTSTRAPS 10000

int compare_longs_asc(const void *a, const void *b) {
    long long arg1 = *(const long long *)a;
    long long arg2 = *(const long long *)b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

int compare_longs_desc(const void *a, const void *b) {
    long long arg1 = *(const long long *)a;
    long long arg2 = *(const long long *)b;
    if (arg1 > arg2) return -1;
    if (arg1 < arg2) return 1;
    return 0;
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
    double *bootstrap_means = malloc(num_bootstraps * sizeof(double));
    if (bootstrap_means == NULL) {
        perror("Fehler beim Allozieren des Speichers");
        exit(1);
    }

    for (int i = 0; i < num_bootstraps; i++) {
        long long *bootstrap_sample = malloc(size * sizeof(long long));
        if (bootstrap_sample == NULL) {
            perror("Fehler beim Allozieren des Speichers");
            exit(1);
        }

        for (int j = 0; j < size; j++) {
            int random_index = rand() % size;
            bootstrap_sample[j] = latencies[random_index];
        }

        bootstrap_means[i] = calculate_median(bootstrap_sample, size);
        free(bootstrap_sample);
    }

    // Sortiere die Bootstrap-Mittelwerte
    qsort(bootstrap_means, num_bootstraps, sizeof(double), compare_longs_asc);

    *ci_lower = bootstrap_means[(int)(num_bootstraps * 0.025)];
    *ci_upper = bootstrap_means[(int)(num_bootstraps * 0.975)];

    free(bootstrap_means);
}

int main() {
    long long latencies[NUM_EXPERIMENTS];
    FILE *file = fopen(FILENAME, "r");
    if (file == NULL) {
        perror("Fehler beim Öffnen der Datei");
        exit(1);
    }

    for (int i = 0; i < NUM_EXPERIMENTS; i++) {
        fscanf(file, "%lld", &latencies[i]);
    }

    fclose(file);

    // Finde die 3 höchsten und 3 niedrigsten Werte
    qsort(latencies, NUM_EXPERIMENTS, sizeof(long long), compare_longs_desc);
    printf("3 höchste Latenzen: %lld ns, %lld ns, %lld ns\n", latencies[0], latencies[1], latencies[2]);

    qsort(latencies, NUM_EXPERIMENTS, sizeof(long long), compare_longs_asc);
    printf("3 niedrigste Latenzen: %lld ns, %lld ns, %lld ns\n", latencies[0], latencies[1], latencies[2]);

    double median = calculate_median(latencies, NUM_EXPERIMENTS);
    double ci_lower, ci_upper;
    bootstrap_confidence_interval(latencies, NUM_EXPERIMENTS, NUM_BOOTSTRAPS, &ci_lower, &ci_upper);

    printf("Median der Latenz: %.2f ns\n", median);
    printf("Konfidenzintervall (95%%): [%.2f ns, %.2f ns]\n", ci_lower, ci_upper);

    return 0;
}
