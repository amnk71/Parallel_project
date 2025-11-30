# Parallel Radix Sort (OpenMP)

## High-Performance Integer Sorting Using Multi-Threading

### 🧩 Overview

This project contains a **parallel Radix Sort implementation in C using OpenMP**.
The program reads integers from a `.txt` dataset, sorts them using **LSD Radix Sort**, and accelerates the digit-counting and copying stages using **parallel for-loops with reductions**.

It also **records and prints performance statistics**, compares sequential vs parallel execution times, and automatically logs the results in `OpenMP_output_log3.txt`.

---

## ⚙️ Key Features

* ✔️ Parallelized digit counting using **OpenMP reduction**
* ✔️ Parallelized copy-back step for improved performance
* ✔️ Handles **up to 2,000,000 integers**
* ✔️ Supports negative integers using a shifting technique
* ✔️ Logs results and profiling to an output file
* ✔️ Computes:

  * Sequential time (loaded from lookup table)
  * Parallel time
  * **Speedup**
  * **Efficiency**
  * **Estimated parallel fraction (α)**
  * **Amdahl’s Law predicted speedup**

---

## 📁 Input Format

Input files are simple `.txt` datasets with integers separated by spaces.
Example:

```
12 -4 99 120 -55 3 807 14
```

Datasets can be generated using your Python random generator.

---

## 🛠️ How the Parallel Algorithm Works

### **1. Reading Input**

* Reads integers using `fscanf`
* Stores them in a dynamic array (`malloc`)
* Tracks:

  * total count
  * minimum value
  * maximum value

### **2. Handling Negative Values**

Radix Sort requires non-negative numbers, so the code:

* Computes `shift = -min`
* Adds `shift` to all values
* Restores original values after sorting

### **3. Parallel Counting Sort**

The heavy part of Radix Sort is counting each digit.
This loop is parallelized:

```c
#pragma omp parallel for reduction(+:count[:10])
for (int i = 0; i < n; i++) {
    int digit = (arr[i] / exp) % 10;
    count[digit]++;
}
```

The `reduction` clause ensures **race-free accumulation**.

### **4. Parallel Copy-Back**

After placing elements in `output[]`, copying back is done in parallel:

```c
#pragma omp parallel for
for (int i = 0; i < n; i++)
    arr[i] = output[i];
```

### **5. Performance Profiling**

The program:

* Reads the corresponding sequential time using
  `get_sequential_time(filename)`
* Computes:

  * **Speedup** = T_seq / T_par
  * **Efficiency** = Speedup / Number_of_threads
  * **α (parallel fraction)** from speedup
  * **Amdahl’s predicted speedup**

### **6. Automatic Logging**

For every dataset, results are appended to:

```
OpenMP_output_log3.txt
```

Including:

* Full arrays (if ≤ 100 values)
* Summary for large datasets
* Timing results
* Speedup / Efficiency
* Amdahl prediction

Example log snippet from your output :

```
===== Performance Profiling =====
Sequential Time (T_seq): 0.109000 s
Parallel Time   (T_par): 0.071000 s
Speedup         (S): 1.5352
Efficiency      (E): 0.0960
Amdahl Predicted Speedup: 1.0346
```

---

## 🧪 Example Performance

Our results show that:

* Small datasets → parallel slower (due to overhead)
* Large datasets (1,000,000 integers) → **parallel faster**

### Example (1,000,000 integers)

```
Sequential Time: 0.109000 s
Parallel Time:   0.071000 s
Speedup: 1.5352
Efficiency: 0.0960
```

This confirms that parallelization benefits large workloads significantly.

---

## 📦 File Overview

| File                     | Description                                                            |
| ------------------------ | ---------------------------------------------------------------------- |
| `radix_sort_parallel.c`  | Full OpenMP-parallel Radix Sort implementation with profiling.         |
| `OpenMP_output_log3.txt` | Log file containing sorted output + profiling for all input datasets.  |

---

## 📝 Notes & Limitations

* Overhead of thread creation makes parallel slower for very small datasets
* Counting sort is only partially parallelizable
* Efficiency decreases as thread count grows due to Amdahl’s Law
* Best speedups occur for:

  * **Huge datasets**
  * **High digit diversity**
  * **Many independent operations per loop**

