import h5py
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

file_path = "results/mesh_results.h5"
inputs_path = "../src/inputs/inputs.txt"


def parse_inputs(file_path):
    params = {}
    with open(file_path, "r") as f:
        for line in f:
            line = line.strip()
            if line and not line.startswith("#"):  # Ignore comments and empty lines
                if "=" in line:
                    key, value = line.split("=", 1)
                    params[key.strip()] = value.split("#")[0].strip()  # Extract before comment
    return params

def load_hdf5_data(file_path):
    with h5py.File(file_path, "r") as f:
        node_coords = f["node_coords"][:]
        solution = f["solution"][:]
    x = node_coords[:, 0]
    y = node_coords[:, 1]

    return x, y, solution

def plot_3d(x, y, solution, save_path, N, n):
    fig = plt.figure(figsize=(12, 8))
    ax = fig.add_subplot(111, projection="3d")
    
    # Assume x, y, and solution are structured for a regular grid
    X, Y = np.meshgrid(np.unique(x), np.unique(y))
    Z = solution.reshape(X.shape)  # Reshape solution to match X, Y grid

    surface = ax.plot_surface(X, Y, Z, cmap="viridis", edgecolor="none")
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    ax.set_zlabel("Solution")
    ax.set_title(f"Solution (N={N}, n={n})")
    plt.colorbar(surface, label="Solution")
    plt.savefig(save_path)
    plt.show()

def main():
    # Parse inputs.txt for parameters
    inputs = parse_inputs(inputs_path)
    N = int(inputs.get("mesh_size", 10))  # Default to 10 if not found
    n = float(inputs.get("n", 1))        # Default to 1 if not found

    # Load and plot data
    x, y, solution = load_hdf5_data(file_path)
    plot_3d(x, y, solution, f"results/solution_N{N}_n{n}.png", N, n)

if __name__ == "__main__":
    main()
