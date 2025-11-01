# Parallel_project
## Random Integer Dataset Generator

### 🧩 Overview
This Python script (`random_generator.py`) generates random integer datasets for testing and benchmarking sorting algorithms (e.g., Radix Sort).  
It creates three text files—`input_small.txt`, `input_medium.txt`, and `input_large.txt`—each containing **20 randomly generated integers** within different numeric ranges.



### ⚙️ Features
- Generates random integers using the Python `random` library.
- Automatically saves all output files **in the same directory** as the script.
- Each file contains **20 integers**, separated by spaces.
- Uses the current system time as the random seed to ensure different outputs on every run.
- Prints clear confirmation messages when each dataset is generated.



### 🧠 How It Works
1. The script calls `generate_random_integers()` to produce a list of random numbers.
2. Each list is then passed to `save_to_file()`, which writes the numbers into a `.txt` file.
3. All files are saved in the same folder where the Python script is located.
4. You can later use these files as input for your Radix Sort C programs.



### 📂 Output Files
| File Name | Number of Integers | Range (Min–Max) | Description |
|------------|--------------------|------------------|--------------|
| `input_small.txt` | 20 | -500 to 500 | Small test dataset |
| `input_medium.txt` | 20 | -5000 to 5000 | Medium-range dataset |
| `input_large.txt` | 20 | -5,000,000 to 5,000,000 | Large-range dataset |




