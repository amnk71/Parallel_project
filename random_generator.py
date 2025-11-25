# combined_generator.py
# Generates ALL datasets:
# 1) Fixed-range datasets (small, medium, large)
# 2) Mixed-distribution datasets (50% small, 30% medium, 20% huge)

import random
import time
import os


# FIXED RANGE GENERATOR
def generate_random_integers(n, min_val, max_val):
    """Generate a list of n random integers between min_val and max_val."""
    return [random.randint(min_val, max_val) for _ in range(n)]


# -----------------------------------------------------------
# MIXED RANGE GENERATOR
# (50% small, 30% medium, 20% huge)
# -----------------------------------------------------------
def generate_mixed_integers(n):
    arr = []
    for _ in range(n):
        r = random.random()

        if r < 0.5:      # 50% small
            val = random.randint(-1_000, 1_000)
        elif r < 0.8:    # 30% medium
            val = random.randint(-100_000, 100_000)
        else:            # 20% huge
            val = random.randint(-1_000_000_000, 1_000_000_000)

        arr.append(val)

    return arr


# SAVE FUNCTION
def save_to_file(filename, arr):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    filepath = os.path.join(script_dir, filename)

    with open(filepath, "w") as f:
        f.write(" ".join(str(x) for x in arr))

    print(f"✅ Saved {len(arr)} integers to {filepath}")


def main():
    random.seed(time.time())
    print("=== DATA GENERATION STARTED ===\n")

    # FIXED RANGE DATASETS
    datasets = [
        ("input_small.txt",  20, -500,       500),
        ("input_medium.txt", 20, -5000,      5000),
        ("input_large.txt",  20, -5_000_000, 5_000_000),
    ]

    print("📌 Generating fixed-range datasets...")
    for filename, n, mn, mx in datasets:
        arr = generate_random_integers(n, mn, mx)
        save_to_file(filename, arr)

    # MIXED DISTRIBUTION DATASETS
    sizes = [10_000, 100_000, 1_000_000]

    print("\n📌 Generating mixed-distribution datasets...")
    for n in sizes:
        filename = f"input_mixed_{n}.txt"
        arr = generate_mixed_integers(n)
        save_to_file(filename, arr)

    print("\n🎯 All datasets generated successfully!")


if __name__ == "__main__":
    main()
