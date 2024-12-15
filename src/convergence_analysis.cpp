#include "convergence_analysis.h"
#include "mesh_solver.h"
#include <fstream>
#include <iostream>
#include <Eigen/Dense>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <tuple>

using namespace Eigen;

void norm_calc(const MatrixXd &node_coords, const MatrixXi &elements,const VectorXd &u_h, double &L2_norm, double &H1_seminorm,const int n) {
    int N_elements = elements.rows(); 
    int N_nodes = node_coords.rows(); 
    L2_norm = 0.0;
    H1_seminorm = 0.0;
    const double pi = 3.14159265358979323846;
    const double pi_squared = pi * pi;
    const double C1 = -n * std::exp(-2 * pi) / (pi_squared * (std::exp(-2 * pi) + 1));
    const double C2 = -n / (pi_squared * (std::exp(-2 * pi) + 1));

    // Analytical solution when 
    VectorXd u_ref(N_nodes); 
    for (int i = 0; i < N_nodes; ++i) {
        double x = node_coords(i, 0);
        double y = node_coords(i,1);
        u_ref(i) = -0.5*x*x + x + (C1 * std::exp(pi * x) + C2 * std::exp(-pi * x) + n / pi_squared) * std::cos(pi * y); 
    }

    for (int e = 0; e < N_elements; ++e) {
        int n1 = elements(e, 0);
        int n2 = elements(e, 1);
        int n3 = elements(e, 2);

        Vector2d A1 = node_coords.row(n1);
        Vector2d A2 = node_coords.row(n2);
        Vector2d A3 = node_coords.row(n3);

        // Calculate the area of the triangle
        double det_J = std::abs((A2 - A1).x() * (A3 - A1).y() - (A2 - A1).y() * (A3 - A1).x()) / 2.0;

        // Approximate solution on the triangle
        double uh_e[3] = {u_h(n1), u_h(n2), u_h(n3)};
        double ur_e[3] = {u_ref(n1), u_ref(n2), u_ref(n3)};

        // L2 Norm
        double L2_local = det_J * ((uh_e[0] - ur_e[0]) * (uh_e[0] - ur_e[0]) +
                                   (uh_e[1] - ur_e[1]) * (uh_e[1] - ur_e[1]) +
                                   (uh_e[2] - ur_e[2]) * (uh_e[2] - ur_e[2])) / 3.0;

        // H1 Semi-Norm
        // Use simple gradient computation assuming linear interpolation
        Vector2d grad_uh = ((A2 - A3) * uh_e[0] + (A3 - A1) * uh_e[1] + (A1 - A2) * uh_e[2]) / (2.0 * det_J);
        Vector2d grad_ur = ((A2 - A3) * ur_e[0] + (A3 - A1) * ur_e[1] + (A1 - A2) * ur_e[2]) / (2.0 * det_J);

        double H1_local = det_J * (grad_uh - grad_ur).squaredNorm();


        L2_norm += L2_local;
        H1_seminorm += H1_local;
    }

    L2_norm = std::sqrt(L2_norm);
    H1_seminorm = std::sqrt(H1_seminorm);
}



void runConvergenceTests(const std::vector<int> &mesh_divisions, const std::string &log_file, const int n) {
    std::ofstream logFile(log_file);
    logFile << "mesh_size,L2_norm,H1_seminorm\n";

    for (int N : mesh_divisions) {
        MatrixXd node_coords;
        MatrixXi elements;
        generateMesh(N, elements, node_coords); // Generate mesh

        MatrixXd K;
        VectorXd f = computeStiffnessLoad(N, K, node_coords, elements, 0); // Load vector
        SparseMatrix<double> sparseK = K.sparseView();

        ConjugateGradient<SparseMatrix<double>> solver;
        solver.setMaxIterations(1000);
        solver.setTolerance(1e-10);
        solver.compute(sparseK);

        VectorXd u = solver.solve(f); // Solve system

        if (solver.info() != Success) {
            std::cerr << "Solver failed for mesh size " << N << std::endl;
            continue;
        }

        double L2_norm, H1_seminorm;
        norm_calc(node_coords, elements, u, L2_norm, H1_seminorm, n);

        std::cout << "Mesh size: " << N << ", L2 Norm: " << L2_norm << std::endl;
        logFile << N << "," << L2_norm << "," << H1_seminorm << "\n";
    }

    logFile.close();
}

void generateErrorPlot(const std::string &log_file, const std::string &plot_script) {
    int status = std::system(("python3 " + plot_script + " " + log_file).c_str());
    if (status != 0) {
        std::cerr << "Error: Failed to generate plot." << std::endl;
    }
}


void performConvergenceAnalysis(
    const std::vector<int> &mesh_divisions,
    const std::string &log_file,
    const std::string &plot_script,
    const int n) {
    runConvergenceTests(mesh_divisions, log_file,n);
    generateErrorPlot(log_file, plot_script);
}

