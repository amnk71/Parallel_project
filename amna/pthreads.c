#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>

#define NUM_BUCKETS 256
#define NUM_PASSES  4  // 4 bytes (32-bit integers)

// ----------------------------------------------
// Thread context structure
// ----------------------------------------------
typedef struct {
    uint32_t *in;
    uint32_t *out;
    size_t n;
    int T;
    int tid;
    size_t lo, hi;
} ThreadCtx;

// Shared data across threads
static pthread_barrier_t barrier;
static size_t (*local_count)[NUM_BUCKETS] = NULL; 
static size_t global_count[NUM_BUCKETS];
static size_t global_prefix[NUM_BUCKETS];
static size_t (*thread_offset)[NUM_BUCKETS] = NULL;

// ----------------------------------------------
// Utility Functions
// ----------------------------------------------
static void compute_range(size_t n, int T, int tid, size_t *lo, size_t *hi) {
    size_t base = n / T, rem = n % T;
    *lo = tid * base + (tid < rem ? tid : rem);
    *hi = *lo + base + (tid < rem ? 1 : 0);
}

// Simple check for debugging small n (e.g., n=20)
static int is_sorted(uint32_t *arr, size_t n) {
    for (size_t i = 1; i < n; i++)
        if (arr[i - 1] > arr[i]) return 0;
    return 1;
}

// ----------------------------------------------
// Thread Worker Function
// ----------------------------------------------
static void *worker(void *arg) {
    ThreadCtx *ctx = (ThreadCtx*)arg;
    uint32_t *in  = ctx->in;
    uint32_t *out = ctx->out;
    const int T   = ctx->T;
    const int tid = ctx->tid;
    const size_t lo = ctx->lo, hi = ctx->hi;

    for (int pass = 0; pass < NUM_PASSES; ++pass) {
        int shift = pass * 8;

        // 1️⃣ Local counting phase
        for (int b = 0; b < NUM_BUCKETS; ++b)
            local_count[tid][b] = 0;

        for (size_t i = lo; i < hi; ++i) {
            uint32_t x = in[i];
            int d = (x >> shift) & 0xFF;
            local_count[tid][d]++;
        }

        pthread_barrier_wait(&barrier);

        // 2️⃣ Combine counts (only one thread)
        if (tid == 0) {
            for (int b = 0; b < NUM_BUCKETS; ++b) {
                size_t s = 0;
                for (int t = 0; t < T; ++t)
                    s += local_count[t][b];
                global_count[b] = s;
            }

            // 3️⃣ Compute prefix sums
            size_t run = 0;
            for (int b = 0; b < NUM_BUCKETS; ++b) {
                global_prefix[b] = run;
                run += global_count[b];
            }

            // Compute per-thread offsets (race-free write zones)
            for (int b = 0; b < NUM_BUCKETS; ++b) {
                size_t base = global_prefix[b];
                for (int t = 0; t < T; ++t) {
                    thread_offset[t][b] = base;
                    base += local_count[t][b];
                }
            }
        }

        pthread_barrier_wait(&barrier);

        // 4️⃣ Scatter phase (stable write)
        for (size_t i = lo; i < hi; ++i) {
            uint32_t x = in[i];
            int d = (x >> shift) & 0xFF;
            size_t pos = thread_offset[tid][d]++;
            out[pos] = x;
        }

        pthread_barrier_wait(&barrier);

        // 5️⃣ Swap arrays (once per pass)
        if (tid == 0) {
            uint32_t *tmp = ctx->in;
            ctx->in = ctx->out;
            ctx->out = tmp;
        }

        pthread_barrier_wait(&barrier);
        in  = ctx->in;
        out = ctx->out;
    }

    return NULL;
}

// ----------------------------------------------
// Main Function
// ----------------------------------------------
int main(int argc, char **argv) {
    printf("\n--- Parallel Radix Sort (pThreads) ---\n");

    // 0️⃣ Auto-generate new random data before sorting
    printf("Generating new random input using Python script...\n");
    int ret = system("python3 random_generator.py");
    if (ret != 0) {
        fprintf(stderr, "Error: Failed to run random_generator.py\n");
        return 1;
    }

    // 1️⃣ File names (hardcoded for project convenience)
    char *input_file = "input.txt";
    char *output_file = "sorted_pthreads.txt";
    int T = 4; // Default threads (can adjust for testing)

    // 2️⃣ Read input data
    FILE *fp = fopen(input_file, "r");
    if (!fp) { perror("input.txt"); exit(1); }

    size_t n_alloc = 100000; // enough for 20 or more
    uint32_t *A = malloc(n_alloc * sizeof(uint32_t));
    uint32_t *B = malloc(n_alloc * sizeof(uint32_t));
    if (!A || !B) { perror("malloc"); exit(1); }

    int val;
    size_t n = 0;
    while (fscanf(fp, "%d", &val) == 1) {
        A[n++] = (uint32_t)(val + 2147483648u); // handle negatives safely
    }
    fclose(fp);

    printf("Loaded %zu integers from %s\n", n, input_file);

    // 3️⃣ Allocate shared structures
    local_count   = calloc(T, sizeof(*local_count));
    thread_offset = calloc(T, sizeof(*thread_offset));
    if (!local_count || !thread_offset) { perror("calloc"); exit(1); }

    pthread_barrier_init(&barrier, NULL, T);
    pthread_t *threads = malloc(T * sizeof(pthread_t));
    ThreadCtx *ctx = malloc(T * sizeof(ThreadCtx));

    // 4️⃣ Create threads
    for (int t = 0; t < T; ++t) {
        compute_range(n, T, t, &ctx[t].lo, &ctx[t].hi);
        ctx[t].in = A;
        ctx[t].out = B;
        ctx[t].n = n;
        ctx[t].T = T;
        ctx[t].tid = t;
        pthread_create(&threads[t], NULL, worker, &ctx[t]);
    }

    // 5️⃣ Join threads
    for (int t = 0; t < T; ++t)
        pthread_join(threads[t], NULL);

    uint32_t *sorted = (NUM_PASSES % 2 == 0) ? A : B;

    // 6️⃣ Write output
    FILE *out = fopen(output_file, "w");
    if (!out) { perror("output"); exit(1); }
    for (size_t i = 0; i < n; i++) {
        int32_t val_signed = (int32_t)(sorted[i] - 2147483648u);
        fprintf(out, "%d ", val_signed);
    }
    fclose(out);

    // 7️⃣ Verification (optional for 20 elements)
    if (n <= 20) {
        printf("\nUnsorted Input:\n");
        system("cat input.txt");
        printf("\n\nSorted Output:\n");
        system("cat sorted_pthreads.txt");
    }

    if (is_sorted(sorted, n))
        printf("\n✅ Array sorted successfully using %d threads.\n\n", T);
    else
        printf("\n❌ Sorting failed.\n");

    // TODO (later):
    // - Add timing with clock_gettime() for performance graphs
    // - Scale up n and test for 1000, 10000, 1M inputs

    // 8️⃣ Cleanup
    pthread_barrier_destroy(&barrier);
    free(local_count);
    free(thread_offset);
    free(threads);
    free(ctx);
    free(A);
    free(B);

    return 0;
}
