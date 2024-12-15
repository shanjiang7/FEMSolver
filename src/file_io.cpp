
#include "file_io.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "hdf5.h"




// Function to parse the input file
bool parseInputFile(const std::string &filename, int &mesh_division,std::vector<int> &mesh_divisions, int &n, int &max_iter, double &tolerance, int &verbosity) {
    std::ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Unable to open input file: " << filename << std::endl;
        return false;
    }

    std::string line;
    bool inMeshSection = false;
    bool inSolverSection = false;

    while (std::getline(inputFile, line)) {
        // Ignore comments and empty lines
        if (line.empty() || line[0] == '#') continue;

        // Detect sections
        if (line == "[mesh]") {
            inMeshSection = true;
            inSolverSection = false;
            continue;
        }
        if (line == "[solver]") {
            inMeshSection = false;
            inSolverSection = true;
            continue;
        }

        // Parse key-value pairs
        if (line.find('=') != std::string::npos) {
            std::istringstream iss(line);
            std::string key, equalSign;

            // Extract key and equal sign
            iss >> key >> equalSign;

            if (equalSign == "=") {
                if (inMeshSection) {
                    int intValue;
                    if (key == "mesh_division" && iss >> intValue) {
                        mesh_division = intValue;
                    }

                    else if (key == "mesh_divisions") {
                        // Parse mesh_divisions as a list of integers
                        int value;
                        char comma;
                        while (iss >> value) {
                            mesh_divisions.push_back(value);
                            if (iss.peek() == ',') {
                                iss >> comma;  // Skip the comma
                            }
                        }
                        //std::cout << "Parsed mesh_divisions: ";
                        for (int div : mesh_divisions) {
                            std::cout << div << " ";
                        }
                        std::cout << std::endl;
                    }

                } else if (inSolverSection) {
                    if (key == "n") {
                        int intValue;
                        if (iss >> intValue) {
                            n = intValue;
                        }
                    } else if (key == "max_iter") {
                        int intValue;
                        if (iss >> intValue) {
                            max_iter = intValue;
                          // std::cout << "Parsed max_iter: " << max_iter << std::endl;
                        }
                    } else if (key == "tolerance") {
                        double doubleValue;
                        if (iss >> doubleValue) {
                            tolerance = doubleValue;
                          // std::cout << "Parsed tolerance: " << tolerance << std::endl;
                        }
                    } else if (key == "verbosity") {
                        int intValue;
                        if (iss >> intValue) {
                            verbosity = intValue;
                        }
                    }
                }
            }
        }
    }

    inputFile.close();
    return true;
}

// Function to save results using HDF5
bool saveResultsHDF5(int N, const MatrixXd &node_coords, const VectorXd &u, const std::string &filename) {
    // Create a new HDF5 file
    hid_t file_id = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (file_id < 0) {
        std::cerr << "Error: Unable to create HDF5 file." << std::endl;
        return false;
    }

    // Save node coordinates
    hsize_t coords_dims[2] = {static_cast<hsize_t>(node_coords.rows()), static_cast<hsize_t>(node_coords.cols())};
    hid_t coords_space_id = H5Screate_simple(2, coords_dims, NULL);
    hid_t coords_dataset_id = H5Dcreate2(file_id, "node_coords", H5T_NATIVE_DOUBLE, coords_space_id, 
                                         H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (coords_dataset_id < 0) {
        std::cerr << "Error: Unable to create dataset for node coordinates." << std::endl;
        H5Fclose(file_id);
        return false;
    }
    H5Dwrite(coords_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, node_coords.data());

    // Save solution vector
    hsize_t u_dims[1] = {static_cast<hsize_t>(u.size())};
    hid_t u_space_id = H5Screate_simple(1, u_dims, NULL);
    hid_t u_dataset_id = H5Dcreate2(file_id, "solution", H5T_NATIVE_DOUBLE, u_space_id, 
                                    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (u_dataset_id < 0) {
        std::cerr << "Error: Unable to create dataset for solution vector." << std::endl;
        H5Dclose(coords_dataset_id);
        H5Fclose(file_id);
        return false;
    }
    H5Dwrite(u_dataset_id, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, u.data());

    // Close all HDF5 resources
    H5Dclose(coords_dataset_id);
    H5Sclose(coords_space_id);
    H5Dclose(u_dataset_id);
    H5Sclose(u_space_id);
    H5Fclose(file_id);

    std::cout << "Results successfully saved to " << filename << std::endl;
    return true;
}
