[![CI](https://github.com/UT-CSE-380-F2024/group-project-team-5/actions/workflows/test.yaml/badge.svg)](https://github.com/UT-CSE-380-F2024/group-project-team-5/actions/workflows/test.yaml)

# group-project-team-5

## How to Use the Docker Image
This repository provides the `shawnraul7/femsolver` Docker image, a prebuilt environment for running the **FEMSolver** application.

### 1. Pull the Image
To pull the Docker image from Docker Hub, run:
```bash
docker pull shawnraul7/femsolver:latest
```

### 2. Run the Image
To execute the default command specified in the Docker image (e.g., solving a mesh or generating results):
```bash
docker run --rm -it shawnraul7/femsolver
```

### 3.Run the executable file
To run the executable MeshSolver File, go to build folder:
```bash
mkdir build
cd build
cmake ..
make
bin/MeshSolver
```
When you sucessfully run it, you will see 
```bash
Results successfully saved to ../results/mesh_results.h5
Visualization completed successfully!
```

### 4.Use the verification mode
To run the verification mode, add -v at the end:
```bash
bin/MeshSolver -v
```

### 5. Run Test-suite
To run the test-suite, type ctest in the current directory
```bash
ctest
```
The output will be :
```bash
100% tests passed, 0 tests failed out of 3
```

### 5.Check the result
To check the output file and image, go to results folder:
```bash
cd results
ls
```

 ### 6. Valgrind
 We also load valgrind and enable user to check the memory leak:
 ```bash
 cd bin
 valgrind --leak-check=full ./MeshSolver > valgrind_clean_output.txt 2>&1
 cat valgrind_clean_output.txt
 ```

### Contributor
The project, completed during Fall 2024, was a collaborative effort equally contributed to by me, Alwin Anto, Judy Hao, and Bowen Shi, at UT Austin. I utilized this repository to make the project publicly accessible.