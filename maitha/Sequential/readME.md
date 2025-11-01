# ⚙️ Radix Sort (Sequential, Base-10)

This project implements a **Sequential LSD (Least Significant Digit)** ✨ **Radix Sort** in **C**.
It prints:

* 🧾 the **unsorted** array,
* 🔢 the **partial outputs** after each digit pass (ones → tens → hundreds → …),
* ✅ the **final sorted** array,
* ⏱️ and the **time** taken to sort.

Because regular radix sort only works with **non-negative** numbers, the code automatically shifts arrays containing negatives ➕ so that all values become ≥ 0 during sorting, then shifts them back ➖ afterward.

---

## 📂 Files Included

* `radix_sort_sequential.c` → main program (includes timing, printing, and counting sort).
* `input_small.txt`, `input_medium.txt`, `input_large.txt` → must contain **exactly 20 integers** (separated by spaces or newlines).

---

## 🧠 How It Works (Step-by-Step)

1. 📥 **Read 20 integers** from an input file.
2. 🖨️ **Print** the unsorted array.
3. ⚖️ **Shift negatives** if needed:

   * If the smallest number is negative, add `shift = -min` to make all numbers non-negative.
4. 🔄 **Radix loop** — process digits from least to most significant (`exp = 1, 10, 100, ...`):

   * Call `counting_sort()` for each digit.
   * Print the array after each pass.
5. ⏲️ **Measure time** using `clock()` before and after sorting.
6. 🔙 **Shift back** to restore original (negative) values.
7. 🏁 **Print** the final sorted array and total time.

---

## 💻 How to Compile & Run

### 🧱 With GCC

```bash
gcc -O2 -std=c11 -o radix_seq radix_sort_sequential.c
./radix_seq input_small.txt
```

### 🧩 With Code::Blocks

* Create a new **Console Application**.
* Add `radix_sort_sequential.c` to the project.
* Set *Program arguments* → `input_small.txt`.
* ✅ Build and Run.

> ⚠️ The code expects **exactly 20 integers** in your input file.
> If fewer are present, it exits with an error message.

---

## ➕ Handling Negative Numbers

Radix sort can’t directly handle negative values.
To fix that, the program shifts all numbers **upward** before sorting and shifts them **back down** afterward.

Example:

```
Original:  -406  -140   30   372
Shift = 496
Shifted:     90   356  526   868
```

After sorting, the same `shift` is subtracted to restore the original range.

---

## 🧮 Radix Sort + Counting Sort Explained

Radix sort works by sorting **one digit at a time**, starting from the **ones place**, then **tens**, then **hundreds**, etc.
Each digit pass uses **Counting Sort**, which is:

* Linear-time (O(n)) for small ranges (digits 0–9).   --> Not Sure.
* **Stable**, meaning equal digits keep their input order — this is critical for radix sort to work.

---

## 🔍 Step-by-Step Example of Counting Sort

Let’s take:

```
arr = [170, 45, 75, 90, 802, 24, 2, 66]
```

and sort by the **ones digit** (`exp = 1`).

### Step 1️⃣ — Count digit frequencies

We count how many numbers have each ones digit:

| Digit |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |
| :---- | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: |
| Count |  2  |  0  |  2  |  0  |  1  |  2  |  1  |  0  |  0  |  0  |

Example:
`170` and `90` end with 0,
`802` and `2` end with 2, etc.

---

### Step 2️⃣ — Convert to cumulative counts

We update the counts so each value shows the **ending position** (1-based) for that digit.

```
Before: [2, 0, 2, 0, 1, 2, 1, 0, 0, 0]
After : [2, 2, 4, 4, 5, 7, 8, 8, 8, 8]
```

This means:

* Digit 0 → positions 1–2
* Digit 2 → positions 3–4
* Digit 4 → position 5
* Digit 5 → positions 6–7
* Digit 6 → position 8

---

### Step 3️⃣ — Place elements (right ➡️ left)

We go **right-to-left** through `arr[]` to keep the order of equal digits (stability).

For each number:

1. Find its current digit.
2. Use `count[d] - 1` as its **index** in `output`.
3. Place it there.
4. Decrease `count[d]`.

Result after sorting by ones:

```
[170, 90, 802, 2, 24, 45, 75, 66]
```

Ones digits: `0, 0, 2, 2, 4, 5, 5, 6` ✅

---

### Step 4️⃣ — Copy back

Finally, we copy `output[]` into `arr[]`,
so the next digit pass (tens, hundreds, etc.) continues from this partially sorted array.

---

## 🧩 Why We Go Right → Left

After prefix sums, each `count[d]` represents the **ending position** for digit `d`.
Placing items **from right to left** ensures that equal digits appear in the same order they were in originally — making the sort **stable**.

(You could also compute **starting positions** and go **left to right**, but that’s a different version of counting sort.)

---

## ⏱️ Time Complexity   -> Not Sure.

| Step                      | Complexity       |
| ------------------------- | ---------------- |
| Counting sort (per digit) | O(n + 10) ≈ O(n) |
| Total radix sort          | O(k · n)         |
| Space                     | O(n + 10)        |

Where:

* `n` = number of elements (20 here)
* `k` = number of digits in the largest number

---

## 🧪 Example Output

```
Unsorted: -406 -453 -137 -140 165 372 -406 30 ...

After pass for exp = 1:
... partially sorted by ones digit

After pass for exp = 10:
... partially sorted by tens digit

After pass for exp = 100:
... partially sorted by hundreds digit

Sorted:
-453 -406 -406 -140 -137 30 165 372 ...
Sorting Time: 0.001234 s
```

If negative numbers exist, the **partial outputs** might show their **shifted positive equivalents**, but the final output restores the correct values.

---

## 🧰 Common Issues

| Problem                               | Cause                                              | Fix                                         |
| ------------------------------------- | -------------------------------------------------- | ------------------------------------------- |
| ❌ `Error: expected 20 integers`       | Input file has fewer than 20 numbers               | Add or correct numbers in `input_small.txt` |
| 🤔 Partial outputs show weird numbers | Those are **shifted** values (to handle negatives) | Final “Sorted” output shows real numbers    |
| ⏱️ Time always shows 0                | Input is too small (20 numbers is too fast)        | Use larger dataset or repeat sort in a loop |

---

## 🚀 Possible Extensions

* 🔹 Handle dynamic input sizes (use `malloc()` for `arr` and `output`)
* 🔹 Parallelize counting phase with **OpenMP**
* 🔹 Support variable digit bases (e.g., base-2, base-16)
* 🔹 Add command-line flags for number of integers, base, etc.

---

## 🧩 Key Functions

| Function                     | Description                                                               |
| ---------------------------- | ------------------------------------------------------------------------- |
| `print_array(label, arr, n)` | Prints a label and the array elements                                     |
| `counting_sort(arr, n, exp)` | Sorts by a single digit using counting sort (stable)                      |
| `main()`                     | Reads data, shifts negatives, calls radix sort, times, and prints results |

---

✨ **In short:**
This program demonstrates how radix sort works internally — by combining multiple stable counting sort passes over each digit — while keeping the logic simple, efficient, and easy to trace using printed intermediate results.

