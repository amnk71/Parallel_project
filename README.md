# Parallel_Project

## Advanced Random Integer Dataset Generator

### 🧩 Overview

This project includes an enhanced Python script (`combined_generator.py`) that generates **multiple types of random integer datasets** for testing, benchmarking, and profiling high-performance sorting algorithms (such as Radix Sort, parallel Radix Sort with OpenMP, pthreads, or multiprocessing).

The generator produces **two categories** of dataset files:

1. **Fixed-range datasets** (small, medium, large) — each containing **20 integers**
2. **Mixed-distribution datasets** (10k, 100k, 1M integers) — ideal for stress-testing parallel programs

All files are automatically saved in the **same directory** as the script.

---

### ⚙️ Features

* ✔️ Generates integers across multiple controlled ranges
* ✔️ Includes a **mixed-distribution generator** (50% small, 30% medium, 20% huge values)
* ✔️ Ideal for benchmarking CPU threads, OpenMP scheduling, or memory-intensive experiments
* ✔️ Uses system time as the seed for unique outputs
* ✔️ Creates datasets from **20 values** up to **1,000,000 values**
* ✔️ Produces clean `.txt` files with space-separated integers
* ✔️ Prints confirmation messages for all generated files

---

### 🧠 How It Works

#### **1. Fixed-range generator**

Uses `generate_random_integers()` to create:

* Small range
* Medium range
* Large range
  Each file has **20 integers**, perfect for debugging or small test cases.

#### **2. Mixed-range generator**

Uses `generate_mixed_integers()` to generate large datasets where:

* **50%** of numbers are between **–1,000 and 1,000**
* **30%** are between **–100,000 and 100,000**
* **20%** are between **–1,000,000,000 and 1,000,000,000**

This produces realistic, diverse workloads for performance evaluation.

#### **3. Automatic saving**

The `save_to_file()` function stores all datasets in the same folder as the script and prints a success message for each generated file.

---

### 📂 Output Files

#### **Fixed-Range Datasets (20 integers each)**

| File Name          | Count | Range                   | Purpose                                  |
| ------------------ | ----- | ----------------------- | ---------------------------------------- |
| `input_small.txt`  | 20    | –500 to 500             | Simple dataset for debugging             |
| `input_medium.txt` | 20    | –5000 to 5000           | Moderate test dataset                    |
| `input_large.txt`  | 20    | –5,000,000 to 5,000,000 | Large-range values for edge-case testing |

#### **Mixed-Distribution Datasets**

| File Name                 | Count     | Distribution                    | Purpose                                               |
| ------------------------- | --------- | ------------------------------- | ----------------------------------------------------- |
| `input_mixed_10000.txt`   | 10,000    | 50% small, 30% medium, 20% huge | Medium-scale benchmarking                             |
| `input_mixed_100000.txt`  | 100,000   | Same distribution               | Multithreading stress tests                           |
| `input_mixed_1000000.txt` | 1,000,000 | Same distribution               | Full performance testing, profiling, caching analysis |

---

### 📎 Source File

This README corresponds to the uploaded script:
**`combined_generator.py`** 


