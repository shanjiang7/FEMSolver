#define BOOST_TEST_MODULE MeshSolverTests
#include <boost/test/included/unit_test.hpp>
#include <Eigen/Dense>
#include "mesh_solver.h"
#include "file_io.h"
#include "convergence_analysis.h"
#include <iostream>
#include <fstream>


// Test case for parseInputFile
BOOST_AUTO_TEST_CASE(test_parse_input_file) {
    int mesh_division = 0;
    int n = 0;
    int max_iter = 1000;
    double tolerance = 1e-10;
    int verbosity = 0;   
    std::vector<int> mesh_divisions = {5, 10, 25, 50, 100};

    // Write the input file with the new format
    std::ofstream input_file("inputs_test.txt");
    input_file << "[mesh]\n";
    input_file << "mesh_division = 30\n";
    input_file << "[solver]\n";
    input_file << "n = 0\n";
    input_file << "notify_freq = 10\n";
    input_file << "max_iter = 1000\n";
    input_file << "tolerance = 1e-7\n";
    input_file << "verbosity = 2\n";
    input_file.close();

    // Test input parsing with the new test file
    BOOST_CHECK(parseInputFile("inputs_test.txt", mesh_division, mesh_divisions, n, max_iter, tolerance,verbosity));
    
    // Check that the values were parsed correctly
    BOOST_CHECK_EQUAL(mesh_division, 30);
    BOOST_CHECK_EQUAL(n, 0);
    BOOST_CHECK_EQUAL(max_iter, 1000);
    BOOST_CHECK_EQUAL(tolerance, 1e-7);
    BOOST_CHECK_EQUAL(mesh_divisions.size(), 5);
    BOOST_CHECK_EQUAL(mesh_divisions[0], 5);
    BOOST_CHECK_EQUAL(mesh_divisions[1], 10);
}

// Test case for computing L2 norm and comparing it to the expected value
BOOST_AUTO_TEST_CASE(test_solver_L2_norm) {
    int N = 10;              
    int n = 0;               
    int max_iter = 1000;     
    double tolerance = 1e-10; 
    double expected_L2_norm = 0.004228; // Expected L2 norm for mesh size 1/10
    

    // Generate the mesh
    MatrixXd node_coords;
    MatrixXi elements;
    generateMesh(N, elements, node_coords); // Generate mesh

    // Compute stiffness matrix and load vector
    MatrixXd K;
    VectorXd f = computeStiffnessLoad(N, K, node_coords, elements, n);
    SparseMatrix<double> sparseK = K.sparseView();

    // Solve the system
    ConjugateGradient<SparseMatrix<double>> solver;
    solver.setMaxIterations(max_iter);
    solver.setTolerance(tolerance);
    solver.compute(sparseK);

    VectorXd u = solver.solve(f); // Solve system

    if (solver.info() != Success) {
        std::cerr << "Solver failed for mesh size " << N << std::endl;
        BOOST_FAIL("Solver failed");
    }

    
    VectorXd u_ref(node_coords.rows());  

    const double pi = 3.14159265358979323846;
    const double pi_squared = pi * pi;
    const double C1 = -n * std::exp(-2 * pi) / (pi_squared * (std::exp(-2 * pi) + 1));
    const double C2 = -n / (pi_squared * (std::exp(-2 * pi) + 1));

    // Compute u_ref based on node coordinates
    for (int i = 0; i < node_coords.rows(); ++i) {
        double x = node_coords(i, 0);
        double y = node_coords(i, 1);
        u_ref(i) = -0.5 * x * x + x + (C1 * std::exp(pi * x) + C2 * std::exp(-pi * x) + n / pi_squared) * std::cos(pi * y);
    }

    // Calculate the L2 norm
    double L2_norm = 0.0;
    for (int i = 0; i < u.size(); ++i) {
        L2_norm += (u(i) - u_ref(i)) * (u(i) - u_ref(i));
    }
    L2_norm = std::sqrt(L2_norm);

    // Compare the computed L2 norm with the expected value
    BOOST_CHECK_CLOSE(L2_norm, expected_L2_norm,1); 

    std::cout << "Test passed for mesh size " << N << ", L2 Norm: " << L2_norm << std::endl;
}

