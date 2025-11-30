# Sequential Radix Sort

### 🧩 Overview

This repository contains a **sequential Radix Sort implementation in C**. The program reads integers from a `.txt` file, stores them in memory, and sorts them efficiently using **LSD (Least Significant Digit) Radix Sort** combined with **Counting Sort** as the stable digit-level sorter.

It supports large datasets of up to **2 million integers**, including negative numbers, by applying an automatic shifting technique.

---

### 🚀 Key Features

* ✔️ Pure **sequential** implementation (no parallelization)
* ✔️ Supports up to **2,000,000 integers**
* ✔️ Handles **negative integers** using a shifting method
* ✔️ Uses efficient **Counting Sort** at each digit level
* ✔️ Prints the array before and after sorting (if ≤ 100 elements)
* ✔️ Displays minimum/maximum values for large datasets
* ✔️ Includes precise **execution-time measurement**
* ✔️ Fully commented and beginner-friendly source code

---

### 📁 Input Format

The program expects a **single text file** containing integers separated by spaces.
Example:

```
12 5 900 -10 45 77 -200 304 0
```

You can generate these datasets using your random generator script.

---


### 🧠 How the Algorithm Works

#### **1. Reading the Input File**

* Reads integers one by one using `fscanf()`
* Stores them in a dynamically allocated array (`malloc`)
* Tracks:

  * total count
  * minimum value
  * maximum value

#### **2. Handling Negative Numbers**

If any value is negative, the program **shifts the entire array upward** by adding `|min|` to all numbers.

Example:
If minimum = –300 → shift = 300 → all numbers become non-negative.

This is required because classic Radix Sort only works with non-negative integers.

#### **3. Radix Sort Loop**

For each digit place (`1, 10, 100, 1000, ...`):

* Apply **Counting Sort** for that digit
* Ensure **stability** by inserting elements from right to left
* Stop when all digit places have been processed

#### **4. Restore Original Values**

If shifting was applied earlier, subtract the shift value after sorting to return numbers to their original range.

#### **5. Output**

* Prints sorted array if ≤ 100 values
* Prints overall sorting time

---

### 📊 Example Output (Small Dataset)

```
Unsorted: [12, 900, -10, 45, 7, -3]

 After pass for exp = 1:
[900, -10, -3, 12, 45, 7]

 After pass for exp = 10:
[...]

Sorted: [-10, -3, 7, 12, 45, 900]

Sorting Time: 0.000132 s
```

---

### 📦 File Overview

| File                      | Description                                                                            |
| ------------------------- | -------------------------------------------------------------------------------------- |
| `radix_sort_sequential.c` | Complete sequential Radix Sort implementation with negative-handling and timing logic. |

---

### 📝 Notes

* If the input file contains more than 100 integers, the program **does not print the full array** to avoid overwhelming the terminal.
* Instead, it prints:

  * Number of elements
  * Minimum value
  * Maximum value

---

### 📌 Source File

This README corresponds to the implementation in:
**`radix_sort_sequential.c`** 

---
