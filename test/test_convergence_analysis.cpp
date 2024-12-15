#define BOOST_TEST_MODULE ConvergenceAnalysisTests
#include <boost/test/unit_test.hpp>
#include "convergence_analysis.h"
#include <Eigen/Dense>
#include <fstream>
#include <vector>
#include <string>


BOOST_AUTO_TEST_CASE(TestRunConvergenceAnalysis) {
    // Parameters
    std::vector<int> mesh_divisions = {4, 8}; // Simple test cases
    std::string log_file = "test_convergence_log.csv";
    std::string plot_script = "mock_plot.py";
    int n = 2;

    // Run the analysis
    performConvergenceAnalysis(mesh_divisions, log_file, plot_script, n);

    // Validate log file
    std::ifstream logFile(log_file);
    BOOST_REQUIRE(logFile.is_open());

    std::string header;
    std::getline(logFile, header);
    BOOST_CHECK_EQUAL(header, "mesh_size,L2_norm,H1_seminorm");

    // Validate content for each mesh size
    std::string line;
    int line_count = 0;
    while (std::getline(logFile, line)) {
        std::stringstream ss(line);
        int mesh_size;
        double l2_norm, h1_seminorm;

        char delimiter;
        ss >> mesh_size >> delimiter >> l2_norm >> delimiter >> h1_seminorm;

        BOOST_CHECK(mesh_size > 0);
        BOOST_CHECK(l2_norm >= 0);
        BOOST_CHECK(h1_seminorm >= 0);
        ++line_count;
    }
    BOOST_CHECK_EQUAL(line_count, mesh_divisions.size());

    logFile.close();

    // Clean up
    std::remove(log_file.c_str());
}

BOOST_AUTO_TEST_CASE(TestEdgeCaseEmptyMeshDivisions) {
    std::vector<int> empty_mesh_divisions;
    std::string log_file = "test_empty_mesh_log.csv";
    std::string plot_script = "mock_plot.py";
    int n = 2;

    // Run with empty mesh divisions
    performConvergenceAnalysis(empty_mesh_divisions, log_file, plot_script, n);

    // Validate log file is created but empty after header
    std::ifstream logFile(log_file);
    BOOST_REQUIRE(logFile.is_open());

    std::string header;
    std::getline(logFile, header);
    BOOST_CHECK_EQUAL(header, "mesh_size,L2_norm,H1_seminorm");

    std::string line;
    BOOST_CHECK(!std::getline(logFile, line)); // No additional lines

    logFile.close();

    // Clean up
    std::remove(log_file.c_str());
}

