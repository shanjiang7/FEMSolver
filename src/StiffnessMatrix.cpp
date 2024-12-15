#include "mesh_solver.h"
#include <iostream>

VectorXd computeStiffnessLoad(int N, MatrixXd &K, const MatrixXd &node_coords, const MatrixXi &elements, int n) {
    int M = elements.rows(); // Number of elements
    int num_nodes = (N + 1) * (N + 1); // Total number of nodes
    K.resize(num_nodes, num_nodes); // Global stiffness matrix
    VectorXd f(num_nodes); // Global load vector
    K.setZero();
    f.setZero();

    const double pi = 3.14159265358979323846;

    // Quadrature rule for reference triangle
    std::vector<Eigen::Vector2d> quad_points = {
        {1.0 / 6, 1.0 / 6},
        {2.0 / 3, 1.0 / 6},
        {1.0 / 6, 2.0 / 3}
    };
    std::vector<double> quad_weights = {1.0 / 6, 1.0 / 6, 1.0 / 6}; // Quadrature weights

    for (int e = 0; e < M; ++e) {
        // Get vertex coordinates for element e
        Eigen::Vector2d A1 = node_coords.row(elements(e, 0));
        Eigen::Vector2d A2 = node_coords.row(elements(e, 1));
        Eigen::Vector2d A3 = node_coords.row(elements(e, 2));

        // Compute Jacobian and determinant
        Eigen::Matrix2d Ae;
        Ae.col(0) = A2 - A1;
        Ae.col(1) = A3 - A1;
        double det_Ae = std::abs(Ae.determinant());

        // Local stiffness matrix computation
        Eigen::Matrix<double, 3, 2> K_t;
        Eigen::Matrix<double, 3, 3> K_e;
        K_t << (A2(1) - A3(1)), (A2(0) - A3(0)),
               (A3(1) - A1(1)), (A3(0) - A1(0)),
               (A1(1) - A2(1)), (A1(0) - A2(0));
        K_e = (K_t * K_t.transpose()) / (2 * det_Ae);

        // Local load vector
        Eigen::Vector3d f_e = Eigen::Vector3d::Zero();

        // Compute element load vector using quadrature
        for (size_t q = 0; q < quad_points.size(); ++q) {
            Eigen::Vector2d ref_point = quad_points[q]; // Quadrature point in reference triangle
            double w_q = quad_weights[q];              // Quadrature weight

            // Map to physical coordinates
            Eigen::Vector2d phys_point = Ae * ref_point + A1;
            double x_q = phys_point(0);
            double y_q = phys_point(1);

            // Evaluate h(x, y) = 1 + n * cos(y)
            double h_q = 1 + n * std::cos(pi * y_q);

            // Shape functions at the quadrature point
            Eigen::Vector3d phi_q;
            phi_q(0) = 1 - ref_point(0) - ref_point(1); // Basis function at vertex 1
            phi_q(1) = ref_point(0);                   // Basis function at vertex 2
            phi_q(2) = ref_point(1);                   // Basis function at vertex 3

            // Accumulate contributions to the local load vector
            f_e += w_q * h_q * phi_q;
        }

        // Scale by det_Ae and assemble into the global load vector
        f_e *= det_Ae;
        for (int alpha = 0; alpha < 3; ++alpha) {
            int global_alpha = elements(e, alpha);
            if (global_alpha % (N + 1) == 0) { // Dirichlet condition on the left boundary
                f(global_alpha) = 0;
            } else {
                f(global_alpha) += f_e(alpha);
            }

            // Assemble stiffness matrix
            for (int beta = 0; beta < 3; ++beta) {
                int global_beta = elements(e, beta);
                if (global_beta % (N + 1) == 0 && global_alpha % (N + 1) == 0) {
                    // Dirichlet condition: diagonal entry
                    K(global_alpha, global_beta) = 1.0;
                } else if (global_beta % (N + 1) != 0 && global_alpha % (N + 1) != 0) {
                    // Interior node contributions
                    K(global_alpha, global_beta) += K_e(alpha, beta);
                } else {
                    // Boundary contributions
                    K(global_alpha, global_beta) = 0.0;
                }
            }
        }
    }

    return f; // Return the assembled global load vector
}


// void computeStiffnessLoad(int N, MatrixXd &K, VectorXd &f, const MatrixXd &node_coords, const MatrixXi &elements) {
//     int M = elements.rows(); // Number of elements
//     K.resize((N + 1) * (N + 1), (N + 1) * (N + 1)); // Global stiffness matrix
//     f.resize((N + 1) * (N + 1)); // Global load vector
//     K.setZero();
//     f.setZero();

//     for (int e = 0; e < M; ++e) {
//         // Get vertex coordinates for element e
//         Eigen::Matrix<double, 2, 1> A1, A2, A3;
//         A1 << node_coords(elements(e, 0), 0), node_coords(elements(e, 0), 1);
//         A2 << node_coords(elements(e, 1), 0), node_coords(elements(e, 1), 1);
//         A3 << node_coords(elements(e, 2), 0), node_coords(elements(e, 2), 1);

//         double det_A_e = 1.0/(N*N); // Area of triangle

//         // Compute the element stiffness matrix K^e
//         Eigen::Matrix<double, 3, 2> K_t;
//         Eigen::Matrix<double, 3, 3> K_e;
//         K_t << (A2(1) - A3(1)), (A2(0) - A3(0)),
//                (A3(1) - A1(1)), (A3(0) - A1(0)),
//                (A1(1) - A2(1)), (A1(0) - A2(0));
//         K_e =  (K_t * K_t.transpose()) * (N*N) / (2);

//         // Compute the element load vector f^e
//         Eigen::Matrix<double, 3, 1> f_e;
//         f_e << det_A_e / 6, det_A_e / 6, det_A_e / 6; // Uniform load

//         // Assemble into the global stiffness matrix and load vector
//         for (int alpha = 0; alpha < 3; ++alpha) {
//             int global_alpha = elements(e, alpha);
//             if (global_alpha % (N + 1) == 0) {
//                 f(global_alpha) = 0;
//             }
//             else {
//                 f(global_alpha) += det_A_e / 6;
//             }
//             for (int beta = 0; beta < 3; ++beta) {
//                 int global_beta = elements(e, beta);
//                 if (global_beta % (N + 1) == 0 && global_alpha % (N + 1) == 0) {
//                      K(global_alpha, global_beta) = 1;
//                 } 
//                 else if (global_beta % (N + 1) != 0 && global_alpha % (N + 1) != 0) {
//                      K(global_alpha, global_beta) += K_e(alpha, beta);
//                 }
//                 else {
//                     K(global_alpha, global_beta) = 0;
//                 }
//             }
            
//         }
//     }
// }
