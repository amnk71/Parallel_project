#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#define THREADS 4
#define DIGITS 10

typedef struct {
    int *arr;
    int *output;
    int start;
    int end;
    int exp;
    int local_count[DIGITS];
} ThreadData;

// Shared globals
pthread_mutex_t lock;
pthread_cond_t cond;
int thread_done = 0;
int total_threads;

// Barrier (simple version)
void barrier_init(int n) {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    total_threads = n;
}
void barrier_wait() {
    pthread_mutex_lock(&lock);
    thread_done++;
    if (thread_done >= total_threads) {
        thread_done = 0;
        pthread_cond_broadcast(&cond);
    } else {
        pthread_cond_wait(&cond, &lock);
    }
    pthread_mutex_unlock(&lock);
}

// Thread worker: parallel counting for one digit pass
void *worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    for (int i = 0; i < DIGITS; i++) data->local_count[i] = 0;

    for (int i = data->start; i < data->end; i++) {
        int digit = (data->arr[i] / data->exp) % 10;
        data->local_count[digit]++;
    }

    barrier_wait(); // sync after counting
    return NULL;
}

// Counting sort by digit (parallel)
void parallel_counting_sort(int *arr, int n, int exp) {
    pthread_t threads[THREADS];
    ThreadData tdata[THREADS];
    int chunk = n / THREADS;
    int output[n];

    // 1. Parallel counting
    barrier_init(THREADS);
    for (int t = 0; t < THREADS; t++) {
        tdata[t].arr = arr;
        tdata[t].exp = exp;
        tdata[t].start = t * chunk;
        tdata[t].end = (t == THREADS - 1) ? n : (t + 1) * chunk;
        pthread_create(&threads[t], NULL, worker, &tdata[t]);
    }
    for (int t = 0; t < THREADS; t++)
        pthread_join(threads[t], NULL);

    // 2. Merge counts
    int global_count[DIGITS] = {0};
    for (int t = 0; t < THREADS; t++)
        for (int d = 0; d < DIGITS; d++)
            global_count[d] += tdata[t].local_count[d];

    for (int d = 1; d < DIGITS; d++)
        global_count[d] += global_count[d - 1];

    // 3. Stable placement
    for (int i = n - 1; i >= 0; i--) {
        int digit = (arr[i] / exp) % 10;
        output[--global_count[digit]] = arr[i];
    }

    // 4. Copy back
    for (int i = 0; i < n; i++)
        arr[i] = output[i];
}

// Sequential radix logic (shared)
void radix_sort_parallel(int *arr, int n) {
    int min = INT_MAX;
    for (int i = 0; i < n; i++)
        if (arr[i] < min) min = arr[i];
    if (min < 0)
        for (int i = 0; i < n; i++) arr[i] -= min;

    int max = arr[0];
    for (int i = 1; i < n; i++)
        if (arr[i] > max) max = arr[i];

    for (int exp = 1; max / exp > 0; exp *= 10)
        parallel_counting_sort(arr, n, exp);

    if (min < 0)
        for (int i = 0; i < n; i++) arr[i] += min;
}

// File loader
int *read_input(const char *filename, int *n) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("open");
        return NULL;
    }
    int *arr = malloc(1000 * sizeof(int));
    int count = 0;
    while (fscanf(f, "%d", &arr[count]) == 1) count++;
    fclose(f);
    *n = count;
    return arr;
}

int main() {
    const char *inputs[] = {"input_small.txt", "input_medium.txt", "input_large.txt"};
    const int num_files = 3;

    FILE *log = fopen("performance_results_pthread.txt", "a"); // append mode

    // Add timestamp header
    time_t now = time(NULL);
    char *dt = ctime(&now);
    if (dt[strlen(dt)-1] == '\n') dt[strlen(dt)-1] = '\0';
    fprintf(log, "============================================\n");
    fprintf(log, "Run Timestamp: %s\n", dt);
    fprintf(log, "Threads used: %d\n", THREADS);
    fprintf(log, "============================================\n\n");


    for (int f = 0; f < num_files; f++) {
        const char *file = inputs[f];
        printf("\n[Dataset: %s]\n", file);

        // Run sequential reference
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "./radix_seq %s > tmp.txt", file);
        system(cmd);

        double seq_time = 0.0;
        FILE *tmp = fopen("tmp.txt", "r");
        if (tmp) {
            char line[256];
            while (fgets(line, sizeof(line), tmp))
                if (sscanf(line, "Sorting Time: %lf", &seq_time) == 1)
                    break;
            fclose(tmp);
        }

        int n;
        int *arr = read_input(file, &n);
        if (!arr) continue;

        struct timespec t1, t2;
        clock_gettime(CLOCK_MONOTONIC, &t1);
        radix_sort_parallel(arr, n);
        clock_gettime(CLOCK_MONOTONIC, &t2);
        double par_time = (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec)/1e9;

        printf("Sorted Output:\n");
        for (int i = 0; i < n; i++)
            printf("%d ", arr[i]);
        printf("\n");   

        double S = seq_time / par_time;
        double E = S / THREADS;
        double alpha = (S - 1) / (THREADS - 1);

        fprintf(log, "==== Dataset: %s ====\n", file);
        fprintf(log, "Sequential time: %.6f s\n", seq_time);
        fprintf(log, "Parallel time:   %.6f s\n", par_time);
        fprintf(log, "Speedup (S):     %.2fx\n", S);
        fprintf(log, "Efficiency (E):  %.2f\n", E);
        fprintf(log, "Amdahl’s α:      %.2f\n", alpha);
        fprintf(log, "--------------------------------------------\n\n");


        free(arr);
        remove("tmp.txt");
    }

    fclose(log);
    printf("\nFull report saved to performance_results_pthread.txt\n");
    return 0;
}
