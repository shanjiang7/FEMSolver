#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "mesh_solver.h"
#include "hdf5.h"
#include "convergence_analysis.h"
#include "file_io.h"

int main(int argc, char *argv[]) {


    int N = 4;                   // Default number of divisions for the mesh
    int n = 0;                   // Default value of n
    int max_iter = 1000;         // Default value for max iterations
    int verbosity = 0;           // Default verbosity
    double tolerance = 1e-10;    // Default value for solver tolerance
    std::vector<int> mesh_divisions = {5,10,25,50,100};  // Default mesh_divisions for convergence analysis

    // Read the mesh size and n from input file
    if (!parseInputFile("../src/inputs/inputs.txt", N, mesh_divisions, n, max_iter, tolerance,verbosity)) {
        std::cerr << "Using default values. For user inputting change values in src/inputs/inputs.txt " << std::endl;
    } else { if(verbosity>0) {
	std::cout << "Values read from input file - " <<std::endl;
        std::cout << "Mesh divisions : " << N << std::endl;
    	std::cout << "Parameter n of f=1+ncos(pi*y) : " << n << std::endl;
	std::cout << "Maximum iterations used by CG solver: " << max_iter <<std::endl;
	std::cout << "Tolerance used by the solver: " << tolerance << std::endl;
    }
    }

    if (argc > 1 && (std::string(argv[1]) == "--verification" || std::string(argv[1]) == "-v")) {
        std::cout << "Verification mode enabled!" << std::endl; 
        std::string log_file = "results/convergence_log.csv";
        std::string plot_script = "../src/plot_code/plot_convergence.py";
        performConvergenceAnalysis(mesh_divisions, log_file, plot_script,n);

        return 0;

    }

    MatrixXd node_coords; // Node coordinates
    MatrixXi elements;    // Element connectivity

    generateMesh(N, elements, node_coords); // Generate mesh

    MatrixXd K; // Stiffness matrix

    // Compute stiffness matrix and load vector
    VectorXd f = computeStiffnessLoad(N, K, node_coords, elements, n);

    SparseMatrix<double> sparseK = K.sparseView(); // Convert to sparse matrix format

    ConjugateGradient<SparseMatrix<double>> solver; // Conjugate Gradient solver
    solver.setMaxIterations(max_iter);
    solver.setTolerance(tolerance);
    solver.compute(sparseK);

    VectorXd u = solver.solve(f); // Solve system

    if (solver.info() != Success) {
        std::cerr << "Conjugate Gradient solver failed to converge!" << std::endl;
        return -1; // Exit if convergence fails
    }

    // Save results to HDF5 file
    std::string output_file = "results/mesh_results.h5";
    saveResultsHDF5(N, node_coords, u, output_file);

    // Call the Python script for visualization
    std::string python_script = "../src/plot_code/plot_mesh.py";
    std::string command = "python3 " + python_script;
    int status = std::system(command.c_str());
    if (status != 0) {
        std::cerr << "Error: Failed to execute Python script: " << python_script << std::endl;
        return -1;
    }

    std::cout << "Visualization completed successfully!" << std::endl;

    return 0;
}
