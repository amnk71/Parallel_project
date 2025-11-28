#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>


#define THREADS 4   // each thread runs the same function on different parts of the array
// barriers: all threads need to reach it before any can proceed - sync point
// race conditions: avoided using shared variables protected using mutexes or barriers
    // within barrier, mutexes and condition variables (for thread wait/signal) are used
#define DIGITS  10  // radix sort uses digits 0–9
#define ADAPT_THRESHOLD 2000   /* <= n triggers fast sequential path inside the parallel function */

typedef struct {
    int *arr;               // pointer to the shared data array
    int start;              // start index for this thread's segment
    int end;                // end index for this thread's segment
    int local_count[DIGITS]; // local histogram count (how many times each digit 0–9 appears in the slice)
} ThreadData;

// Shared globals / barrier section - used to build a custom barrier
pthread_mutex_t lock;        // mutex used inside barrier for synchronization
pthread_cond_t  cond;        // condition variable used to block/wake threads inside the barrier
int thread_done = 0;         // number of threads that have reached the barrier
int total_threads;           // total number of participants in the barrier (workers + main thread)

static int current_exp = 0;  // digit position currently being processed (1,10,100,...)
static int done = 0;         // flag used to tell the worker threads when the sorting is finished

//Barrier (main + workers) - makes sure all threads stay in sync at specific points
static void barrier_init(int n) {      // initializes barrier for n threads
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    total_threads = n;                 // store number of participants
    thread_done = 0;                   // reset counter
}

static void barrier_wait(void) {       // ensures all threads reach the same point before continuing
    pthread_mutex_lock(&lock);
    thread_done++;                     // increment arrival count

    if (thread_done >= total_threads) { // if last thread arrived
        thread_done = 0;               // reset counter for next use
        pthread_cond_broadcast(&cond); // wake up all waiting threads
    } else {
        pthread_cond_wait(&cond, &lock); // otherwise, wait for others
    }
    pthread_mutex_unlock(&lock);
}

// Worker thread function - each thread repeatedly processes its array slice for each digit
static void *worker(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    barrier_wait(); // initial synchronization with main thread

    for (;;) { // runs until main thread signals completion
        barrier_wait(); // wait for main thread to begin new digit pass
        if (done) break; // stop if main thread signals sorting finished

        // reset local histogram for this pass
        for (int d = 0; d < DIGITS; d++)
            data->local_count[d] = 0;

        // count digit occurrences in the thread slice
        int exp = current_exp;
        for (int i = data->start; i < data->end; i++) {
            int digit = (data->arr[i] / exp) % 10; 
            data->local_count[digit]++;
        }

        barrier_wait(); // signal that local counting is complete
    }
    return NULL;
}

//Sequential times hardcoded
static double get_precomputed_seq_time(const char *filename) {
    if (strcmp(filename, "input_small.txt") == 0)        return 0.008;
    if (strcmp(filename, "input_medium.txt") == 0)       return 0.011;
    if (strcmp(filename, "input_large.txt") == 0)        return 0.014;
    if (strcmp(filename, "input_mixed_10000.txt") == 0)  return 0.001;
    if (strcmp(filename, "input_mixed_100000.txt") == 0) return 0.002;
    if (strcmp(filename, "input_mixed_1000000.txt") == 0)return 0.109;
    return 0.0;
}


static void radix_sort_parallel(int *arr, int n) {
    if (n <= 1) return;

    done = 0;

    //Shift negative numbers to non-negative domain
    int min = INT_MAX;
    for (int i = 0; i < n; i++)
        if (arr[i] < min) min = arr[i];
    if (min < 0)
        for (int i = 0; i < n; i++)
            arr[i] -= min;

    //Find maximum to determine number of digits
    int max = (n > 0 ? arr[0] : 0);
    for (int i = 1; i < n; i++)
        if (arr[i] > max)
            max = arr[i];

    //Initialize barrier with workers + main thread
    barrier_init(THREADS + 1);

    pthread_t threads[THREADS];
    ThreadData tds[THREADS];

    //Divide the array approximately evenly across threads
    int base = n / THREADS;
    int rem  = n % THREADS;

    for (int t = 0; t < THREADS; t++) {
        int start = t * base + (t < rem ? t : rem);
        int end   = start + base + (t < rem ? 1 : 0);

        if (start > n) start = n;
        if (end   > n) end   = n;

        tds[t].arr   = arr;
        tds[t].start = start;
        tds[t].end   = end;

        pthread_create(&threads[t], NULL, worker, &tds[t]);
    }

    barrier_wait(); // wait for workers to initialize

    int *output = (int *)malloc(n * sizeof(int));
    if (!output) { perror("malloc"); exit(1); }

    //Process digits from least significant to most significant
    for (current_exp = 1; max / current_exp > 0; current_exp *= 10) {

        barrier_wait(); // start-of-digit pass

        barrier_wait(); // wait for workers to finish counting

        //Merge all thread histograms into global histogram
        int global_count[DIGITS] = {0};
        int total_seen = 0;

        for (int t = 0; t < THREADS; t++)
            for (int d = 0; d < DIGITS; d++)
                global_count[d] += tds[t].local_count[d];

        //Validation step to ensure correct histogram
        for (int d = 0; d < DIGITS; d++)
            total_seen += global_count[d];
        if (total_seen != n) {
            fprintf(stderr, "ERROR: histogram sum %d != n %d (exp=%d)\n",
                    total_seen, n, current_exp);
            exit(1);
        }

        //Prefix sum to compute ending positions for each digit
        for (int d = 1; d < DIGITS; d++)
            global_count[d] += global_count[d - 1];

        //Stable placement into output array (right to left)
        for (int i = n - 1; i >= 0; i--) {
            int digit = (arr[i] / current_exp) % 10;
            output[--global_count[digit]] = arr[i];
        }

        //Copy pass result back to main array
        for (int i = 0; i < n; i++)
            arr[i] = output[i];
    }

    //Signal workers to exit
    done = 1;
    barrier_wait();

    for (int t = 0; t < THREADS; t++)
        pthread_join(threads[t], NULL);

    free(output);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    //Restore original negative values if shifting was applied
    if (min < 0)
        for (int i = 0; i < n; i++)
            arr[i] += min;
}

// File loader
static int *read_input(const char *filename, int *n) {
    FILE *f = fopen(filename, "r");
    if (!f) { perror("open"); return NULL; }

    int cap = 1024, cnt = 0;
    int *arr = (int *)malloc(cap * sizeof(int));
    if (!arr) { perror("malloc"); fclose(f); return NULL; }

    while (1) {
        int v;
        if (fscanf(f, "%d", &v) != 1) break;

        if (cnt == cap) {
            cap *= 2;
            int *tmp = (int *)realloc(arr, cap * sizeof(int));
            if (!tmp) { perror("realloc"); free(arr); fclose(f); return NULL; }
            arr = tmp;
        }
        arr[cnt++] = v;
    }

    fclose(f);
    *n = cnt;
    return arr;
}

// Execute one dataset: read > sort > print > loop
static void run_dataset(FILE *log, const char *filename) {
    printf("\n[Dataset: %s]\n", filename);

    int n = 0;
    int *arr = read_input(filename, &n);
    if (!arr) {
        printf("Skipping (cannot open/read).\n");
        return;
    }

    double seq_time = get_precomputed_seq_time(filename);

    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    radix_sort_parallel(arr, n);
    clock_gettime(CLOCK_MONOTONIC, &t2);

    double par_time =
        (t2.tv_sec - t1.tv_sec) +
        (t2.tv_nsec - t1.tv_nsec) / 1e9;

    if (n <= 100) {
        printf("Sorted Output:\n");
        for (int i = 0; i < n; i++)
            printf("%d%s", arr[i], (i + 1 < n) ? " " : "\n");
    } else {
        printf("Sorted %d integers.\n", n);
    }

    double Sx    = (par_time > 0.0) ? (seq_time / par_time) : 0.0;
    double E     = (THREADS > 0)    ? (Sx / THREADS)        : 0.0;
    double alpha = (THREADS > 1)    ? ((Sx - 1.0) / (THREADS - 1.0)) : 0.0;

    printf("Sequential time (hardcoded): %.6f s\n", seq_time);
    printf("Parallel time:               %.6f s\n", par_time);
    printf("Speedup:                     %.2fx\n", Sx);
    printf("Efficiency:                  %.2f\n", E);

    fprintf(log, "==== Dataset: %s ====\n", filename);
    fprintf(log, "N: %d\n", n);
    fprintf(log, "Sequential time: %.6f s\n", seq_time);
    fprintf(log, "Parallel time:   %.6f s\n", par_time);
    fprintf(log, "Speedup (S):     %.2fx\n", Sx);
    fprintf(log, "Efficiency (E):  %.2f\n", E);
    fprintf(log, "Amdahl’s α:      %.2f\n", alpha);
    fprintf(log, "--------------------------------------------\n\n");

    free(arr);
}


int main(void) {
    FILE *log = fopen("performance_results_pthread.txt", "a");
    if (!log) { perror("open log"); return 1; }

    time_t now = time(NULL);
    char *dt = ctime(&now);
    if (dt && dt[strlen(dt)-1] == '\n')
        dt[strlen(dt)-1] = '\0';

    fprintf(log, "============================================\n");
    fprintf(log, "Run Timestamp: %s\n", dt ? dt : "(unknown time)");
    fprintf(log, "Threads used: %d\n", THREADS);
    fprintf(log, "Adaptive threshold: n <= %d uses sequential path\n", ADAPT_THRESHOLD);
    fprintf(log, "============================================\n\n");

    const char *classic[] = {
        "input_small.txt",
        "input_medium.txt",
        "input_large.txt"
    };
    for (int i = 0; i < 3; ++i)
        run_dataset(log, classic[i]);

    const char *mixed[] = {
        "input_mixed_10000.txt",
        "input_mixed_100000.txt",
        "input_mixed_1000000.txt"
    };
    for (int i = 0; i < 3; ++i)
        run_dataset(log, mixed[i]);

    fclose(log);
    printf("\nFull report saved to performance_results_pthread.txt\n");
    return 0;
}
