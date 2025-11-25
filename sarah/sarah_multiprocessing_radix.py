import os
import time
from multiprocessing import Pool, cpu_count
import heapq

# -------------------------------------------------
# This function reads numbers from a text file.
# The .txt files are in the PARENT folder of sarah/
# (the main Parallel_project folder).
# It returns a Python list of integers.
# -------------------------------------------------
def read_input(filename):
    """
    Read all integers from the given text file.
    The file has numbers separated by spaces.
    Returns them as a Python list of ints.
    """
    # Folder where THIS file (sarah_multiprocessing_radix.py) is located
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Go one level up: parent folder that holds the .txt files
    parent_dir = os.path.dirname(script_dir)

    # Build full path to the input file in the parent folder
    filepath = os.path.join(parent_dir, filename)

    # Open the file and read all content as a string
    with open(filepath, "r") as f:
        content = f.read().strip()

    # If the file is empty, return an empty list
    if not content:
        return []

    # Split the string by spaces and convert each part to int
    numbers = [int(x) for x in content.split()]
    return numbers


# ---------- RADIX SORT FUNCTIONS ----------
# These functions do the normal (sequential) radix sort.
# We will use them both in single-process and multi-process versions.
# -------------------------------------------------


# -------------------------------------------------
# One "digit pass" of radix sort using counting sort.
# Only works for NON-NEGATIVE numbers.
# exp = 1 -> ones, exp = 10 -> tens, etc.
# -------------------------------------------------
def counting_sort_for_digit(arr, exp, base=10):
    """
    One step of radix sort: stable counting sort using the digit at 'exp'.
    """
    n = len(arr)
    output = [0] * n       # result after this digit pass
    count = [0] * base     # how many times each digit appears (0–9)

    # 1) count how many numbers have each digit
    for num in arr:
        digit = (num // exp) % base
        count[digit] += 1

    # 2) convert counts to prefix sums (positions)
    for i in range(1, base):
        count[i] += count[i - 1]

    # 3) fill output from right to left to keep stability
    for i in range(n - 1, -1, -1):
        digit = (arr[i] // exp) % base
        count[digit] -= 1
        output[count[digit]] = arr[i]

    return output


# -------------------------------------------------
# Normal radix sort for NON-NEGATIVE numbers.
# -------------------------------------------------
def radix_sort_non_negative(arr):
    """Standard LSD radix sort for non-negative integers."""
    if not arr:
        return arr

    result = arr[:]          # copy original list
    max_val = max(result)    # largest number
    exp = 1

    # Keep sorting by next digit while max_val still has digits
    while max_val // exp > 0:
        result = counting_sort_for_digit(result, exp)
        exp *= 10

    return result


# -------------------------------------------------
# Radix sort that works with NEGATIVE and POSITIVE integers.
# -------------------------------------------------
def radix_sort(arr):
    """
    Radix sort that works with negative and positive integers.
    """
    if not arr:
        return arr

    # Separate negatives and non-negatives
    negatives = [-x for x in arr if x < 0]
    non_negatives = [x for x in arr if x >= 0]

    # Sort each group with non-negative radix sort
    sorted_neg_abs = radix_sort_non_negative(negatives)
    sorted_non_neg = radix_sort_non_negative(non_negatives)

    # Rebuild negatives: reverse and add minus sign back
    sorted_neg = [-x for x in reversed(sorted_neg_abs)]

    # Final sorted result: negatives first, then non-negatives
    return sorted_neg + sorted_non_neg


# ---------- END RADIX SORT FUNCTIONS ----------


# -------------------------------------------------
# MULTIPROCESSING version of radix sort.
# -------------------------------------------------
def parallel_radix_sort(arr, num_processes=None):
    """
    Parallel version of radix sort.
    Steps:
      1. Split the array into chunks.
      2. Each process sorts its chunk using the normal radix_sort().
      3. Merge all sorted chunks into one sorted list.
    """
    if not arr:
        return arr

    # If not specified, use number of CPU cores
    if num_processes is None:
        num_processes = cpu_count()

    # Don't create more processes than elements
    num_processes = max(1, min(num_processes, len(arr)))

    # Split array into almost equal chunks
    chunk_size = (len(arr) + num_processes - 1) // num_processes
    chunks = [arr[i:i + chunk_size] for i in range(0, len(arr), chunk_size)]

    # Each process sorts one chunk using radix_sort
    with Pool(processes=num_processes) as pool:
        sorted_chunks = pool.map(radix_sort, chunks)

    # Merge all sorted chunks into one big sorted list
    merged = list(heapq.merge(*sorted_chunks))

    return merged


# -------------------------------------------------
# MAIN FUNCTION
# -------------------------------------------------
def main():
    # List of all input files we want to test.
    # These files are in the PARENT folder of sarah/
    input_files = [
        "input_small.txt",
        "input_medium.txt",
        "input_large.txt",
        "input_mixed_10000.txt",
        "input_mixed_100000.txt",
        "input_mixed_1000000.txt"
    ]

    # Loop over each file name in the list
    for filename in input_files:
        # Read numbers from the file
        data = read_input(filename)

        print(f"\n===== {filename} =====")
        print(f"Total numbers read: {len(data)}")

        # --- Sequential radix sort (single process) ---
        start = time.perf_counter()           # start timer
        seq_sorted = radix_sort(data)        # run normal radix sort
        end = time.perf_counter()             # end timer
        seq_time = end - start
        print(f"Sequential radix sort time: {seq_time:.6f} seconds")

        # --- Multiprocessing radix sort (many processes) ---
        start = time.perf_counter()
        mp_sorted = parallel_radix_sort(data)
        end = time.perf_counter()
        mp_time = end - start
        print(f"Multiprocessing radix sort time: {mp_time:.6f} seconds")

        # --- Correctness check ---
        if seq_sorted == mp_sorted:
            print("✅ Both versions produce the SAME result.")
        else:
            print("❌ MISMATCH between sequential and multiprocessing results!")


# -------------------------------------------------
# This runs main() when we execute this file directly.
# Needed for multiprocessing on Windows.
# -------------------------------------------------
if __name__ == "__main__":
    main()
