#define BOOST_TEST_MODULE ComputeStiffnessLoadTests
#include <boost/test/included/unit_test.hpp>
#include "mesh_solver.h" 
#include <Eigen/Dense>

BOOST_AUTO_TEST_SUITE(ComputeStiffnessLoadSuite)

BOOST_AUTO_TEST_CASE(TestBasicStiffnessLoad) {
    using namespace Eigen;

    // Define a simple mesh: unit square with two triangular elements
    MatrixXd node_coords(4, 2); // Node coordinates
    node_coords << 0, 0,
                   1, 0,
                   0, 1,
                   1, 1;

    MatrixXi elements(2, 3); // Two elements
    elements << 0, 1, 2,
                1, 3, 2;

    // Test parameters
    int N = 1;       
    int n = 2;       

    // Matrices to hold results
    MatrixXd K;      
    VectorXd f;      

    // Call the function
    f = computeStiffnessLoad(N, K, node_coords, elements, n);

    // Validate stiffness matrix dimensions
    BOOST_CHECK_EQUAL(K.rows(), (N + 1) * (N + 1));
    BOOST_CHECK_EQUAL(K.cols(), (N + 1) * (N + 1));

    // Validate load vector dimensions
    BOOST_CHECK_EQUAL(f.size(), (N + 1) * (N + 1));

}



BOOST_AUTO_TEST_SUITE_END()

