# random_generator.py
# Generates an array (list) of 20 random integers and writes them to input.txt.
# You can reuse this function in Sequential, MPI, or multiprocessing programs.

import random
import time

def generate_random_integers(n, min_val, max_val):
    """Generate a list of n random integers between min_val and max_val."""
    # Seed the random number generator with the current time
    random.seed(time.time())

    # Create the random integer list
    arr = [random.randint(min_val, max_val) for _ in range(n)]
    return arr


def main():
    # Number of integers to generate
    n = 20

    # Value range for this dataset
    min_val = -500
    max_val = 500

    # Generate random numbers
    arr = generate_random_integers(n, min_val, max_val)

    # Overwrite the same file each time for new random input
    with open("input.txt", "w") as f:
        f.write(" ".join(str(x) for x in arr))

    print(f"Generated {n} random integers and saved to input.txt.")
    print(" ".join(str(x) for x in arr))


if __name__ == "__main__":
    main()
