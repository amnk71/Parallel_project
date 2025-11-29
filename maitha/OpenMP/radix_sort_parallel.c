// radix_sort_parallel.c
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <omp.h>                    //Include OpenMP header.
#define MAX_NUMS 2000000            //Maximum number of integers the program can store in the dynamic array.



// -------- Load sequential time based on filename --------
double get_sequential_time(const char *fname) {
    if (strcmp(fname, "input_small.txt") == 0)   return 0.008;
    if (strcmp(fname, "input_medium.txt") == 0)  return 0.011;
    if (strcmp(fname, "input_large.txt") == 0)   return 0.014;
    if (strcmp(fname, "input_mixed_10000.txt") == 0)    return 0.001;
    if (strcmp(fname, "input_mixed_100000.txt") == 0)   return 0.020;
    if (strcmp(fname, "input_mixed_1000000.txt") == 0)  return 0.109;

    // default fallback
    return 0.0;
}


// ---------- Function to print an array ----------
void print_array(const char *label, int *arr, int n) {         //This Function is used to print a label followed by all array elements.
    printf("%s[", label);                                       //Printing the provided label string (e.g., "Unsorted: ").
    for (int i = 0; i < n; i++) {                              //Iterating over each element from index 0 to 19.
        if (i > 0)                                             //Printing commas from second element onward.
            printf(", ");
        printf("%d", arr[i]);                                  //Printing the current integer element.
    }
    printf("]\n");
}



// ---------- Counting sort ----------
void counting_sort(int *arr, int n, int exp) {                //Sort array elements based on the digit represented by exp (1, 10, 100...).
    int *output = malloc(n * sizeof(int));                 //Creating a temporary array to store the sorted result for this digit.
    if (!output) {
        fprintf(stderr, "Memory allocation failed in counting_sort\n");
        exit(1);
    }

    int count[10] = {0};                                      //Array to count occurrences of each digit (0–9) initialized to zero.

    // ---------- PARALLEL COUNTING USING REDUCTION ----------
    #pragma omp parallel for reduction(+:count[:10])
    for (int i = 0; i < n; i++) {
        int digit = (arr[i] / exp) % 10;
        count[digit]++;
    }


     //2)Converting counts into 1-based ending positions for each digit.
    for (int i = 1; i < 10; i++) {                            //Looping through the count array starting from index 1 to 9.
            int prev = count[i - 1];                          //Storing the cumulative count of all digits less than the current digit i.
            int curr = count[i];                             //Storing the current count of the digit i.
            count[i] = curr + prev;                          //Updating count[i] to be the total number of elements with a digit value <= i.
        }

    //3)Placing items into output[] from right to left using those positions.
    for (int i = n - 1; i >= 0; i--) {                        //Traversing the array backward to maintain sorting stability.
        int d = (arr[i] / exp) % 10;                          //Extracting the current digit again.
        int pos = count[d] - 1;                               //Converting last 1-based position to 0-based index.
        output[pos] = arr[i];                                  //Placing the element into that exact position in the output array for this digit.
        count[d]--;                                            //Decreasing the count so the next same digit element goes to the left.
    }


    //4)Copying the partially sorted result back to the main array arr[].
    #pragma omp parallel for                                  //Splitting the loop among threads for faster copying.
    for (int i = 0; i < n; i++)                               //Copying the sorted elements from output[] back to arr[].
        arr[i] = output[i];                                   //Updating the original array with partially sorted result.


    free(output);

}


// ---------- Main ----------
int main(int argc, char *argv[]) {
  // --- Check that exactly one argument (input file name) is provided ---
    if (argc != 2) {                                           //If user didn’t pass exactly one extra argument notify them the correct format.
        printf("Usage: %s <input.txt>\n", argv[0]);
        return 1;                                              //Exiting with error.
    }

    // --- Open the input file ---
    FILE *f = fopen(argv[1], "r");                             //Trying to open the file given as the first argument in read mode.
    if (!f) {                                                  //If fopen failed print the system error message.
        perror("fopen");
        return 1;                                              //Exiting with error.
    }

    // --- Reading the array from file ---
    int *arr = malloc(MAX_NUMS * sizeof(int));              //Declaring a dynamic array to hold all integers read from the file.
    if (!arr) {
        fprintf(stderr, "Memory allocation failed (arr)\n");
        fclose(f);
        return 1;
    }
    int n = 0;                                          //Initializing a counter that keeps track of how many integers have been successfully read.
    int x;                                              //Declaring a temporary variable used to store each integer read from the file.
     int min = INT_MAX;                                 //Initializing a variable to track the smallest value seen so far, initialized to the largest int.
    int max_read = INT_MIN;   // Track max while reading

    while (fscanf(f, "%d", &x) == 1 && n < MAX_NUMS) {    //Keep reading integers from the file while read is successful and array is not full.
        arr[n] = x;                                      //Storing the new integer at the current index n.
        n++;                                             //Moving n to the next position for the next read.
        if (x < min)
            min = x;                            //If this value is smaller than the current minimum, update min.
        if (x > max_read)
            max_read = x;
    }

    if (n == 0) {                                        //If no integers were read (n stayed 0)...
        fprintf(stderr, "Error: no integers found.\n");   //Print an error message to the standard error stream.
        fclose(f);                                       //Close the file before exiting.
        free(arr);
        return 1;                                        //Exit the program with an error status.
    }
    fclose(f);

    printf("Number of threads: %d\n", omp_get_max_threads());

    // --- Print unsorted array or summary ---
     // ---- Make a copy of unsorted array for logging ----
    int *unsorted_copy = NULL;
    if (n <= 100) {
        unsorted_copy = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++)
            unsorted_copy[i] = arr[i];
        print_array("Unsorted:", arr, n);
    } else {
        printf("Unsorted array is too large to print fully.\n");
        printf("  Number of elements: %d\n", n);
        printf("  Minimum value:      %d\n", min);
        printf("  Maximum value:      %d\n", max_read);
        printf("\n");
    }

    // --- Shift negatives (Sequential)---
        int shift = 0;                                              //Initializing a variable that stores how much we need to add to each number.
        if (min < 0)                                                //Checking if the smallest number in the array is negative.
            shift = -min;                                           //If yes, make shift equal to its positive value (so negatives become zero or more).

        if (shift > 0) {                                            //If shifting is needed (there were negative numbers).
            for (int i = 0; i < n; i++)                             //Looping through every element in the array.
                arr[i] = arr[i] + shift;                            //Adding 'shift' to each element to make all numbers non negative
        }


    // --- Find maximum (Sequential) ---
    int max = arr[0];                                           //Assuming the first element is the largest number.
    for (int i = 1; i < n; i++)                                 //Looping through the rest of the array from index 1 to 19.
        if (arr[i] > max)                                       //If the current number is larger than the current 'max'.
            max = arr[i];                                       //Updating 'max' to store this new larger number.

    // --- Start timing ---
    double start = omp_get_wtime();

    // --- Radix sort (with partial outputs) ---
    for (int exp = 1; max / exp > 0; exp *= 10) {               //Looping over each digit place: 1 (ones), 10 (tens), 100 (hundreds),etc.
        counting_sort(arr, n, exp);                             //Sorting the array based on the current digit using counting sort.

    if (n <= 100) {
        printf("\n After pass for exp = %d:\n", exp);         //Printing which digit place we just sorted.
        print_array("", arr, n);                              //Printing the array after this pass.
    }
    }

    // --- End timing ---
    double end = omp_get_wtime();
    double time_taken = end - start;                            //Calculating how long the sorting took in seconds.

    // --- Shift back (restore negatives) ---
    if (shift != 0)                                             //If shift is not zero, meaning we actually shifted earlier
        for (int i = 0; i < n; i++)                             //Looping through every element in the array.
            arr[i] -= shift;                                    //Subtracting the same shift value to return numbers to their original range.

    // --- Final output ---
     if (n <= 100) {
        print_array("\nSorted:", arr, n);
    }

    // === Performance Profiling ===
    // Manually enter the sequential time from your sequential program
    double T_seq = get_sequential_time(argv[1]);
    double T_par = time_taken;

    // Compute speedup: S = T_seq / T_par
    double speedup = (T_seq > 0) ? (T_seq / T_par) : 0;

    // Compute efficiency: E = S / P
    int P = omp_get_max_threads();
    double efficiency = (P > 0) ? (speedup / P) : 0;

    //Estimating parallel fraction alpha.
    double alpha = (P > 1) ? ((speedup - 1.0) / (P - 1.0)) : 0.0;

    //Amdahl predicted speedup using estimated alpha
    double amdahl_speedup = 1.0 / ((1.0 - alpha) + (alpha / P));


    printf("\n===== Performance Profiling =====\n");
    printf("\nSequential Time (T_seq): %.6f s\n", T_seq);
    printf("Parallel Time   (T_par): %.6f s\n", T_par);
    printf("Speedup         (S = T_seq / T_par): %.4f\n", speedup);
    printf("Efficiency      (E = S / P): %.4f\n", efficiency);
    printf("Amdahl Predicted Speedup: %.4f\n", amdahl_speedup);
    printf("\n=================================\n\n");

    // === Save output to file ===
    FILE *out = fopen("OpenMP_output_log3.txt", "a");
    if (!out) {
        perror("fopen");
    } else {

        fprintf(out, "=============================================\n");
        fprintf(out, "Run for input file: %s\n", argv[1]);
        fprintf(out, "=============================================\n\n");

        // ---- Case 1: Small arrays: print full unsorted & sorted ----
        if (n <= 100) {
            fprintf(out, "Unsorted Array:\n");
            for (int i = 0; i < n; i++) {
                fprintf(out, "%d ", unsorted_copy[i]);   // use copy here
            }
            fprintf(out, "\n\n");

            fprintf(out, "Sorted Array:\n");
            for (int i = 0; i < n; i++) {
                fprintf(out, "%d ", arr[i]);             // sorted result
            }
            fprintf(out, "\n\n");
        }
        // ---- Case 2: Large arrays: print only summary ----
        else {
            fprintf(out, "Unsorted array is too large to print.\n");
            fprintf(out, "  Number of elements: %d\n", n);
            fprintf(out, "  Minimum value:      %d\n", min);
            fprintf(out, "  Maximum value:      %d\n\n", max_read);
        }

        // ---- Performance Profiling ----
        fprintf(out, "===== Performance Profiling =====\n");
        fprintf(out, "Sequential Time (T_seq): %.6f s\n", T_seq);
        fprintf(out, "Parallel Time   (T_par): %.6f s\n", T_par);
        fprintf(out, "Speedup         (S): %.4f\n", speedup);
        fprintf(out, "Efficiency      (E): %.4f\n", efficiency);
        fprintf(out,"Amdahl Predicted Speedup: %.4f\n", amdahl_speedup);
        fprintf(out, "=============================================\n\n");

        fclose(out);
    }

    if (unsorted_copy)
        free(unsorted_copy);

    free(arr);
    return 0;
}
