#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>

#define THREADS 4   //each run the same fn on different parts of array
// barriers: all threads need to reach it before any can proceed - sync point
// race cond: to avoid them, e used shared variables using mutexes or barriers (mutexes used inside barriers) 
    // within barrier, mutexes and conitiional variables(for thread wait/signal) used)
#define DIGITS  10  //we use 0-9 digits to sort for radix sort
#define ADAPT_THRESHOLD 2000   /* <= n triggers fast sequential path in "parallel" function */

typedef struct {
    int *arr;   // pointer to the shared data array
    int start;  
    int end;    // start and end of indexes for this thread
    int local_count[DIGITS]; // checks count of how many times 0-9 appear in thread slice (from ones to higher digits)
    // ex: [123,243,344,456], to check ones: we check last digsit of each number: 3,3,4,6 -> local_count[3]=2, local_count[4]=1, local_count[6]=1
} ThreadData;

// Shared globals / barrier - used to build custom barrier
pthread_mutex_t lock;   //mutex for barrier
pthread_cond_t  cond;   //condition variable for barrier (wait/signal threads)
int thread_done = 0;    // # of threads that reached the barrier
int total_threads;      // total # of threads participating in barrier

static int current_exp = 0;   // digit position currently being sorted (1,10,100,...)
static int done = 0;          // flag telling when to stop workers, so they exit safely

//Barrier (main + workers) - makes sure all threads stay in sync at certain points
static void barrier_init(int n) {   //initialize barrier for n threads
    pthread_mutex_init(&lock, NULL); 
    pthread_cond_init(&cond, NULL);
    total_threads = n; // sets # of threads that must meet total_threads=n 
    thread_done = 0; // resets thread_done count
}
static void barrier_wait(void){    // purpose: makes sure all the threads reach the same point before continuing
    // each thread locks the mutex and increments thread_done count
    pthread_mutex_lock(&lock);
    thread_done++;
    if (thread_done >= total_threads) { // if thread is last to arive, reset count and wake all waiting threads
        thread_done = 0;
        pthread_cond_broadcast(&cond);
    } else {
        pthread_cond_wait(&cond, &lock); // otherwise, wait on the cond variable until last thread wakes everyone
    }
    pthread_mutex_unlock(&lock); // ADDED: always unlock (fixes potential deadlock)
}

// Worker thread 
static void *worker(void *arg) { // this fn runs with everytime pthread_create is called (making the worker threads)
    ThreadData *data = (ThreadData *)arg;

    /* Initial handshake so all participants start aligned */
    barrier_wait();

    for (;;) { // like while(true) - runs until 'done' flag is set
        /* Phase 1: start-of-pass */
        barrier_wait(); // wait for main to start the pass (dig position)
        if (done) break; // exit if flag is set
        // main thread sets current_exp (dig position) and signals thread to start

        /* Reset local histogram (local_count) */
        for (int d = 0; d < DIGITS; d++) data->local_count[d] = 0;

        /* Count my slice for this digit */
        int exp = current_exp;// all threads work on the same digit till they all finish
        for (int i = data->start; i < data->end; i++) {
            // start and end is where that specific thread starts (0) and ends (2) as ex
            // next thread start where the prev ends+1 so 3-6 as ex
            int digit = (data->arr[i] / exp) % 10; // get the specific digit at curr position
            data->local_count[digit]++; // specific digit appears local_count[digit] times in this thread slice
        }

        /* Phase 2: counting complete */
        barrier_wait(); // wait for all threads to finish counting before next pass (dig position)
    }
    return NULL;
}

// Sequential radix helpers (shared)
static void seq_counting_sort(int *arr, int n, int exp) {
    int count[DIGITS] = {0}; // like the local_count but for whole array (global)
    int *out = (int *)malloc((n > 0 ? n : 1) * sizeof(int)); //temp array to hold sorted values
    if (!out) { perror("malloc"); exit(1); }

    //Let’s say arr = [329, 457, 657, 839, 436] and exp = 1 (ones place).
    for (int i = 0; i < n; i++) { 
        int digit = (arr[i] / exp) % 10; //
        count[digit]++;
    } 
    for (int d = 1; d < DIGITS; d++) count[d] += count[d - 1];

    for (int i = n - 1; i >= 0; i--) {
        int digit = (arr[i] / exp) % 10;
        out[--count[digit]] = arr[i];
    }
    for (int i = 0; i < n; i++) arr[i] = out[i];
    free(out);
}

static void sequential_radix_inplace(int *arr, int n) {
    if (n <= 1) return;

    /* Shift negatives, track max */
    int min = INT_MAX, mx = INT_MIN;
    for (int i = 0; i < n; i++) {
        if (arr[i] < min) min = arr[i];
        if (arr[i] > mx)  mx  = arr[i];
    }
    int shift = 0;
    if (min < 0) {
        shift = -min;
        for (int i = 0; i < n; i++) arr[i] += shift;
        mx += shift;
    }

    for (int exp = 1; mx / exp > 0; exp *= 10)
        seq_counting_sort(arr, n, exp);

    if (shift) {
        for (int i = 0; i < n; i++) arr[i] -= shift;
    }
}

static double time_sequential_radix(const int *src, int n) {
    if (n <= 1) return 0.0;

    int *arr = (int *)malloc(n * sizeof(int));
    if (!arr) { perror("malloc"); exit(1); }
    for (int i = 0; i < n; i++) arr[i] = src[i];

    int min = INT_MAX, mx = INT_MIN;
    for (int i = 0; i < n; i++) {
        if (arr[i] < min) min = arr[i];
        if (arr[i] > mx)  mx  = arr[i];
    }
    int shift = 0;
    if (min < 0) {
        shift = -min;
        for (int i = 0; i < n; i++) arr[i] += shift;
        mx += shift;
    }

    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);

    for (int exp = 1; mx / exp > 0; exp *= 10)
        seq_counting_sort(arr, n, exp);

    clock_gettime(CLOCK_MONOTONIC, &t2);
    double secs = (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1e9;

    free(arr);
    return secs;
}

/* ---------- Parallel radix (persistent THREADS workers) ---------- */
static void radix_sort_parallel(int *arr, int n) {
    if (n <= 1) return;

    /* === Adaptive trick: for tiny n, skip threads entirely === */
    if (n <= ADAPT_THRESHOLD) {
        sequential_radix_inplace(arr, n);
        return;
    }

    /* Reset stop flag for this run (important across multiple datasets) */
    done = 0;

    /* Shift negatives to non-negative */
    int min = INT_MAX;
    for (int i = 0; i < n; i++)
        if (arr[i] < min) min = arr[i];
    if (min < 0)
        for (int i = 0; i < n; i++) arr[i] -= min;

    /* Find maximum */
    int max = (n > 0 ? arr[0] : 0);
    for (int i = 1; i < n; i++)
        if (arr[i] > max) max = arr[i];

    /* Start workers once; barrier participants = THREADS + 1 (main) */
    barrier_init(THREADS + 1);

    pthread_t threads[THREADS];
    ThreadData tds[THREADS];

    /* Balanced work partition (base + remainder) */
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

    /* Initial handshake: join workers' first barrier_wait() */
    barrier_wait();

    int *output = (int *)malloc(n * sizeof(int));
    if (!output) { perror("malloc"); exit(1); }

    /* Passes: two barriers per pass (start / finish) */
    for (current_exp = 1; max / current_exp > 0; current_exp *= 10) {
        /* Phase 1: start pass */
        barrier_wait();

        /* Phase 2: wait for all workers to finish counting */
        barrier_wait();

        /* Merge all thread-local histograms */
        int global_count[DIGITS] = {0};
        int total_seen = 0; // ADDED: sanity check accumulator

        for (int t = 0; t < THREADS; t++)
            for (int d = 0; d < DIGITS; d++)
                global_count[d] += tds[t].local_count[d];

        // ADDED: verify the merged histogram equals n (detects bad slices or missed counts)
        for (int d = 0; d < DIGITS; d++) total_seen += global_count[d];
        if (total_seen != n) {
            fprintf(stderr, "ERROR: histogram sum %d != n %d (exp=%d)\n",
                    total_seen, n, current_exp);
            // Optional: dump slices or per-thread counts here
            exit(1);
        }

        /* Prefix sums */
        for (int d = 1; d < DIGITS; d++)
            global_count[d] += global_count[d - 1];

        /* Stable placement (single-threaded placement) */
        for (int i = n - 1; i >= 0; i--) {
            int digit = (arr[i] / current_exp) % 10;
            output[--global_count[digit]] = arr[i];
        }

        /* Copy back */
        for (int i = 0; i < n; i++) arr[i] = output[i];
    }

    /* Tell workers to exit and release them */
    done = 1;
    barrier_wait(); /* wake workers to see 'done' and break */

    for (int t = 0; t < THREADS; t++)
        pthread_join(threads[t], NULL);

    free(output);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);

    /* Restore negatives */
    if (min < 0)
        for (int i = 0; i < n; i++) arr[i] += min;
}

/* ---------- File loader ---------- */
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

/* ---------- NEW: load sequential time from seq_output.txt ---------- */
// ADDED: looks up a dataset name in seq_output.txt (same folder) and returns its time.
// Format per line: "<filename> <time>" e.g. "input_small_100.txt 0.000008"
static int load_seq_time_from_file(const char *dataset_name, double *out_time) {
    FILE *f = fopen("seq_output.txt", "r");
    if (!f) {
        // If the file doesn't exist, just say "not found".
        return 0;
    }

    char name[256];
    double t;
    int found = 0;

    // Read lines: each scanf tries to get "<string> <double>"
    while (fscanf(f, "%255s %lf", name, &t) == 2) {
        if (strcmp(name, dataset_name) == 0) {
            *out_time = t;
            found = 1;
            break;
        }
    }

    fclose(f);
    return found; // 1 if found, 0 otherwise
}

/* ---------- One dataset run (prints + logs) ---------- */
static void run_dataset(FILE *log, const char *filename) {
    printf("\n[Dataset: %s]\n", filename);

    int n = 0;
    int *arr = read_input(filename, &n);
    if (!arr) {
        printf("Skipping (cannot open/read).\n");
        return;
    }

    /* Sequential timing: now prefer reading from seq_output.txt */
    double seq_time;
    if (!load_seq_time_from_file(filename, &seq_time)) { // ADDED: file-based lookup
        // Fallback if the dataset isn't in seq_output.txt:
        seq_time = time_sequential_radix(arr, n);        // still have a correct value
    }

    /* Parallel timing (with adaptive early-out for tiny n) */
    struct timespec t1, t2;
    clock_gettime(CLOCK_MONOTONIC, &t1);
    radix_sort_parallel(arr, n);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    double par_time = (t2.tv_sec - t1.tv_sec) + (t2.tv_nsec - t1.tv_nsec) / 1e9;

    /* Output (avoid printing huge arrays) */
    if (n <= 100) {
        printf("Sorted Output:\n");
        for (int i = 0; i < n; i++)
            printf("%d%s", arr[i], (i + 1 < n) ? " " : "\n");
    } else {
        printf("Sorted %d integers.\n", n);
    }

    /* CLI metrics */
    double Sx = (par_time > 0.0) ? (seq_time / par_time) : 0.0;
    double E  = (THREADS > 0) ? (Sx / THREADS) : 0.0;
    double alpha = (THREADS > 1) ? ((Sx - 1.0) / (THREADS - 1.0)) : 0.0;

    printf("Sequential time (from file or fallback): %.6f s\n", seq_time);
    printf("Parallel time:                           %.6f s\n", par_time);
    printf("Speedup:                                 %.2fx\n", Sx);
    printf("Efficiency:                              %.2f\n", E);

    /* Log file */
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

/* ---------- Main driver ---------- */
int main(void) {
    FILE *log = fopen("performance_results_pthread.txt", "a");
    if (!log) { perror("open log"); return 1; }

    time_t now = time(NULL);
    char *dt = ctime(&now);
    if (dt && dt[strlen(dt)-1] == '\n') dt[strlen(dt)-1] = '\0';
    fprintf(log, "============================================\n");
    fprintf(log, "Run Timestamp: %s\n", dt ? dt : "(unknown time)");
    fprintf(log, "Threads used: %d\n", THREADS);
    fprintf(log, "Adaptive threshold: n <= %d uses sequential path\n", ADAPT_THRESHOLD);
    fprintf(log, "============================================\n\n");

    /* 1) Classic 20-int inputs first (if present) */
    const char *classic[] = {"input_small.txt", "input_medium.txt", "input_large.txt"};
    int classic_count = (int)(sizeof(classic) / sizeof(classic[0])); // ADDED: compute number of classic datasets
    for (int i = 0; i < classic_count; ++i) run_dataset(log, classic[i]);

    /* 2) Scaled datasets: input_<label>_<size>.txt */
    // CHANGED: instead of generating many small/medium/large files,
    //          we now only use the three mixed-size datasets you showed.
    const char *scaled[] = {
        "input_mixed_10000.txt",
        "input_mixed_100000.txt",
        "input_mixed_1000000.txt"
    }; // ADDED: explicit list of scaled datasets

    int scaled_count = (int)(sizeof(scaled) / sizeof(scaled[0])); // ADDED
    for (int i = 0; i < scaled_count; ++i) {
        run_dataset(log, scaled[i]); // ADDED
    }

    fclose(log);
    printf("\nFull report saved to performance_results_pthread.txt\n");
    return 0;
}
