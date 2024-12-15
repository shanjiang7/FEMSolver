import numpy as np
import matplotlib.pyplot as plt
import sys

def plot_convergence(log_file):
    # Load data from the log file
    data = np.loadtxt(log_file, delimiter=',', skiprows=1)
    mesh_divisions = data[:, 0]
    h = 1 / mesh_divisions
    L2_norms = data[:, 1]
    H1_seminorms = data[:, 2]

  
    plt.figure(figsize=(8, 6))

    # Plot actual norms
    plt.loglog(h, L2_norms, 'bo-', label=r"$L^2$ Norm")
    #plt.loglog(h, H1_seminorms, 'ro-', label=r"$H^1$ Seminorm")

    # Create theoretical reference lines
    h_ref = np.linspace(min(h), max(h), 100)
    plt.loglog(h_ref, h_ref**2, 'b--', label="O($h^2$) Theoretical (L2)")
    #plt.loglog(h_ref, h_ref, 'r--', label="O($h$) Theoretical (H1)")

    plt.xlabel("($h$)", fontsize=14)
    plt.ylabel("Error Norms", fontsize=14)
    plt.title("Log-Log Convergence Analysis", fontsize=16)
    plt.legend(fontsize=12)
    plt.grid(which="both", linestyle="--", linewidth=0.5)
    plt.tight_layout()

    plt.savefig("results/convergence_plot.png")
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 plot_convergence.py <log_file>")
        sys.exit(1)

    log_file = sys.argv[1]
    plot_convergence(log_file)
