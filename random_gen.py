# random_generator.py
# Generates three input files with random integers of different ranges.

import random
import time
import os

def generate_random_integers(n, min_val, max_val):
    """Generate a list of n random integers between min_val and max_val."""
    random.seed(time.time())
    return [random.randint(min_val, max_val) for _ in range(n)]

def save_to_file(filename, arr):
    """Save a list of integers to a text file in the same directory as this script."""
    script_dir = os.path.dirname(os.path.abspath(__file__))   # directory where script is located
    filepath = os.path.join(script_dir, filename)
    with open(filepath, "w") as f:
        f.write(" ".join(str(x) for x in arr))
    print(f"✅ Saved {len(arr)} integers to {filepath}")

def main():
    # You can change 'n' if you want more or fewer numbers in each set.
    n_each = 20


    datasets = [
        ("input_small.txt", n_each, -500, 500),
        ("input_medium.txt", n_each, -5000, 5000),
        ("input_large.txt", n_each, -5000000, 5000000)
    ]

    for filename, n, min_val, max_val in datasets:
        arr = generate_random_integers(n, min_val, max_val)
        save_to_file(filename, arr)

    print("\n🎯 All input files generated successfully!")

if __name__ == "__main__":
    main()

