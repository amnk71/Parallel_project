//radix_sort_sequential.c

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

// ---------- Function to print an array ----------
void print_array(const char *label, int *arr, int n) {         //This Function is used to print a label followed by all array elements.
    printf("%s", label);                                       //Printing the provided label string (e.g., "Unsorted: ").
    for (int i = 0; i < n; i++) {                              //Iterating over each element from index 0 to 19.
        printf(" %d", arr[i]);                                 //Printing the current integer element.
    }
    printf("\n");
}


// ---------- Counting sort  ----------
void counting_sort(int *arr, int n, int exp) {                //Sort array elements based on the digit represented by exp (1, 10, 100...).
    int output[n];                                            //Creating a temporary array to store the sorted result for this digit.
    int count[10] = {0};                                      //Array to count occurrences of each digit (0–9) initialized to zero.

    //1)Counting how many times each digit appears at this place.
    for (int i = 0; i < n; i++) {                             //Looping through all elements in arr from index 0 to 19.
        int digit = (arr[i] / exp) % 10;                      //Extracting the current digit.
        count[digit]++;                                       //Increasing its count.
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
    for (int i = 0; i < n; i++)                               //Copying the sorted elements from output[] back to arr[].
        arr[i] = output[i];                                   //Updating the original array with partially sorted result.

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
    if (!f) {                                                  // If fopen failed print the system error message.
        perror("fopen");
        return 1;                                              //Exiting with error.
    }

    // --- Read exactly 20 integers from file ---
    int arr[20];                                               //Fixed-size array that will hold exactly 20 integers.
    int n = 20;                                               //We expect to read precisely 20 numbers.
    int min = INT_MAX;                                        //Initializing the minimum counter to the largest int.

    for (int i = 0; i < n; ++i) {                               //Looping exactly 20 times to read 20 integers.
        if (fscanf(f, "%d", &arr[i]) != 1) {                   //If reading an integer fails at any point: report that the file didn’t have 20 ints.
            fprintf(stderr, "Error: expected 20 integers in the file.\n");
            fclose(f);                                         //Closing the file before exiting.
            return 1;                                          //Exiting with error.
        }
        if (arr[i] < min)                                      //If the current number is smaller than the current minimum: Update the minimum.
            min = arr[i];
    }

    fclose(f);                                                 //When we are done reading the file, close it.


    // --- Print unsorted array ---
    print_array("Unsorted:", arr, n);

    // --- Shift negatives ---
    int shift = 0;                                              //Initializing a variable that stores how much we need to add to each number.
    if (min < 0)                                                //Checking if the smallest number in the array is negative.
        shift = -min;                                           //If yes, make shift equal to its positive value (so negatives become zero or more).

    if (shift > 0) {                                            //If shifting is needed (there were negative numbers).
        for (int i = 0; i < n; i++)                             //Looping through every element in the array.
            arr[i] = arr[i] + shift;                            //Adding 'shift' to each element to make all numbers non negative
    }


    // --- Find maximum ---
    int max = arr[0];                                           //Assuming the first element is the largest number.
    for (int i = 1; i < n; i++)                                 //Looping through the rest of the array from index 1 to 19.
        if (arr[i] > max)                                       //If the current number is larger than the current 'max'.
            max = arr[i];                                       //Updating 'max' to store this new larger number.


    // --- Start timing ---
    clock_t start = clock();

    // --- Radix sort (with partial outputs) ---
    for (int exp = 1; max / exp > 0; exp *= 10) {               //Looping over each digit place: 1 (ones), 10 (tens), 100 (hundreds),etc.
        counting_sort(arr, n, exp);                             //Sorting the array based on the current digit using counting sort.

        printf("\n After pass for exp = %d:\n", exp);           //Printing which digit place we just sorted.
        print_array("", arr, n);                                //Printing the array after this pass.
    }

    // --- End timing ---
    clock_t end = clock();
    double s = (double)(end - start) / CLOCKS_PER_SEC;         //Calculating how long the sorting took in seconds.

    // --- Shift back (restore negatives) ---
    if (shift != 0)                                             //If shift is not zero, meaning we actually shifted earlier
        for (int i = 0; i < n; i++)                             //Looping through every element in the array.
            arr[i] -= shift;                                    //Subtracting the same shift value to return numbers to their original range.

    // --- Print sorted array and Time Taken ---
    print_array("\nSorted:", arr, n);
    printf("\nSorting Time: %.6f s\n", s);

    return 0;
}
