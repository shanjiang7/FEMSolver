#include "mesh_solver.h"

void generateMesh(int N, MatrixXi &elements, MatrixXd &coordinates) {
    int numElements = 2 * N * N; // Total number of elements
    int numNodes = (N + 1) * (N + 1); // Total number of nodes

    // Resize the output matrices
    elements.resize(numElements, 3);
    coordinates.resize(numNodes, 2);

    // Initialize coordinates
    int nodeIndex = 0;
    for (int j = 0; j <= N; ++j) {
        for (int i = 0; i <= N; ++i) {
            coordinates(nodeIndex, 0) = static_cast<double>(i) / N; // x-coordinate
            coordinates(nodeIndex, 1) = static_cast<double>(j) / N; // y-coordinate
            nodeIndex++;
        }
    }

    // Initialize elements (triangle connectivity)
    int elemIndex = 0;
    for (int j = 0; j < N; ++j) {
        for (int i = 0; i < N; ++i) {
            int bottomLeft = j * (N + 1) + i;
            int bottomRight = bottomLeft + 1;
            int topLeft = bottomLeft + (N + 1);
            int topRight = bottomRight + (N + 1);

            // First triangle (bottom-left)
            elements(elemIndex, 0) = bottomLeft;
            elements(elemIndex, 1) = bottomRight;
            elements(elemIndex, 2) = topLeft;
            elemIndex++;

            // Second triangle (top-right)
            elements(elemIndex, 0) = bottomRight;
            elements(elemIndex, 1) = topRight;
            elements(elemIndex, 2) = topLeft;
            elemIndex++;
        }
    }
}