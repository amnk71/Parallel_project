import os
import time
from multiprocessing import Pool, cpu_count
import heapq

# -------------------------------------------------
# Read integers from a .txt file (parent of sarah/)
# -------------------------------------------------
def read_input(filename):
    """
    Read all integers from the given text file.
    The file has numbers separated by spaces.
    Returns them as a Python list of ints.
    """
    # Folder where THIS file is located: .../Parallel_project/sarah
    script_dir = os.path.dirname(os.path.abspath(__file__))

    # Parent folder: .../Parallel_project   (where the input_*.txt files are)
    parent_dir = os.path.dirname(script_dir)

    # Full path to the file
    filepath = os.path.join(parent_dir, filename)

    with open(filepath, "r") as f:
        content = f.read().strip()

    if not content:
        return []

    return [int(x) for x in content.split()]


# ---------- RADIX SORT FUNCTIONS ----------

def counting_sort_for_digit(arr, exp, base=10):
    """
    One step of radix sort: stable counting sort using the digit at 'exp'.
    Works for non-negative integers.
    """
    n = len(arr)
    output = [0] * n
    count = [0] * base

    # 1) count digits
    for num in arr:
        digit = (num // exp) % base
        count[digit] += 1

    # 2) prefix sums -> positions
    for i in range(1, base):
        count[i] += count[i - 1]

    # 3) build output from right to left (stable)
    for i in range(n - 1, -1, -1):
        digit = (arr[i] // exp) % base
        count[digit] -= 1
        output[count[digit]] = arr[i]

    return output


def radix_sort_non_negative(arr):
    """Standard LSD radix sort for non-negative integers."""
    if not arr:
        return arr

    result = arr[:]          # copy
    max_val = max(result)
    exp = 1

    while max_val // exp > 0:
        result = counting_sort_for_digit(result, exp)
        exp *= 10

    return result


def radix_sort(arr):
    """
    Radix sort that works with negative and positive integers.
    Strategy:
      - separate negatives and non-negatives
      - sort by absolute value
      - rebuild with negatives first
    """
    if not arr:
        return arr

    negatives = [-x for x in arr if x < 0]
    non_negatives = [x for x in arr if x >= 0]

    sorted_neg_abs = radix_sort_non_negative(negatives)
    sorted_non_neg = radix_sort_non_negative(non_negatives)

    sorted_neg = [-x for x in reversed(sorted_neg_abs)]
    return sorted_neg + sorted_non_neg


# ---------- MULTIPROCESSING VERSION ----------

def parallel_radix_sort(arr, num_processes=None):
    """
    Parallel version of radix sort.
    1. Split array into chunks
    2. Each process sorts a chunk with radix_sort()
    3. Merge sorted chunks
    """
    if not arr:
        return arr

    if num_processes is None:
        num_processes = cpu_count()

    # avoid more processes than elements
    num_processes = max(1, min(num_processes, len(arr)))

    # chunk size and split
    chunk_size = (len(arr) + num_processes - 1) // num_processes
    chunks = [arr[i:i + chunk_size] for i in range(0, len(arr), chunk_size)]

    # sort each chunk in a separate process
    with Pool(processes=num_processes) as pool:
        sorted_chunks = pool.map(radix_sort, chunks)

    # merge all sorted chunks
    merged = list(heapq.merge(*sorted_chunks))
    return merged


# ---------- MAIN ----------

def main():
    # Input datasets to test (files are in the parent folder)
    input_files = [
        "input_small.txt",
        "input_medium.txt",
        "input_large.txt",
        "input_mixed_10000.txt",
        "input_mixed_100000.txt",
        "input_mixed_1000000.txt"
    ]

    max_processes = cpu_count()

    for filename in input_files:
        data = read_input(filename)

        print(f"\n===== {filename} =====")
        print(f"Total numbers read: {len(data)}")

        if not data:
            print("File is empty. Skipping.")
            continue

        # number of processes actually used for this dataset
        used_processes = max(1, min(max_processes, len(data)))

        # ------- Sequential radix sort -------
        start = time.perf_counter()
        seq_sorted = radix_sort(data)
        end = time.perf_counter()
        seq_time = end - start
        print(f"Sequential radix sort time:      {seq_time:.6f} seconds")

        # ------- Parallel radix sort -------
        start = time.perf_counter()
        mp_sorted = parallel_radix_sort(data, num_processes=used_processes)
        end = time.perf_counter()
        mp_time = end - start
        print(f"Multiprocessing radix sort time: {mp_time:.6f} seconds")
        print(f"Processes used: {used_processes}")

        # ------- Correctness check -------
        if seq_sorted == mp_sorted:
            print("Both versions produce the SAME result.")
        else:
            print("WARNING: results are DIFFERENT between sequential and parallel versions.")

        # ------- Speedup and efficiency -------
        if mp_time > 0:
            speedup = seq_time / mp_time
            efficiency = speedup / used_processes
            print(f"Speedup  (T_seq / T_par):        {speedup:.3f}")
            print(f"Efficiency (speedup / p):        {efficiency:.3f}")
        else:
            print("Parallel time too small to measure; cannot compute speedup.")


if __name__ == "__main__":
    main()
