cmake_minimum_required(VERSION 3.10)
project(MeshSolver)

# Add include directories for the header files
include_directories(${CMAKE_SOURCE_DIR}/src)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable profiling flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(status "coverage enabled") 
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Eigen setup
set(EIGEN3_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/eigen-3.4.0")
include_directories(${EIGEN3_INCLUDE_DIR})

# First, try using the TACC environment variables for HDF5
if(DEFINED ENV{TACC_HDF5_INC} AND DEFINED ENV{TACC_HDF5_LIB})
    message(STATUS "Using HDF5 from TACC environment variables.")
    include_directories($ENV{TACC_HDF5_INC})
    set(HDF5_LINK_LIBRARIES
        $ENV{TACC_HDF5_LIB}/libhdf5.so
        $ENV{TACC_HDF5_LIB}/libhdf5_hl.so
    )
else()
    # Local HDF5 setup if TACC environment variables are not set
    find_package(HDF5 REQUIRED COMPONENTS CXX HL)
    if(HDF5_FOUND)
        message(STATUS "Using locally installed HDF5.")
        include_directories(${HDF5_INCLUDE_DIRS})
        link_directories(${HDF5_LIBRARY_DIRS})
        set(HDF5_LINK_LIBRARIES ${HDF5_CXX_LIBRARIES} ${HDF5_HL_LIBRARIES})
    else()
        message(FATAL_ERROR "HDF5 library not found. Please ensure it is installed and accessible.")
    endif()
endif()


# Find Boost for testing
find_package(Boost REQUIRED COMPONENTS unit_test_framework)
if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Boost not found. Please install Boost development libraries.")
endif()

# Source files
set(SOURCES 
    src/GenerateMesh.cpp 
    src/StiffnessMatrix.cpp  
    src/convergence_analysis.cpp 
    src/file_io.cpp
)

# Add executable for main program
add_executable(${PROJECT_NAME} src/main.cpp ${SOURCES})

# Link HDF5 libraries
target_link_libraries(${PROJECT_NAME} ${HDF5_LINK_LIBRARIES})

# Create results directory
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/results)

# Set runtime output directory
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Add Boost Test for computeStiffnessLoad
add_executable(test_computeStiffnessLoad ${CMAKE_SOURCE_DIR}/test/test_computeStiffnessLoad.cpp ${SOURCES})
add_executable(test_convergence_analysis ${CMAKE_SOURCE_DIR}/test/test_convergence_analysis.cpp ${SOURCES})
add_executable(test_main ${CMAKE_SOURCE_DIR}/test/test_main.cpp ${SOURCES})

# Link Boost and HDF5 to the test executables
target_link_libraries(test_computeStiffnessLoad PRIVATE Boost::unit_test_framework ${HDF5_LINK_LIBRARIES})
target_link_libraries(test_convergence_analysis PRIVATE Boost::unit_test_framework ${HDF5_LINK_LIBRARIES})
target_link_libraries(test_main PRIVATE Boost::unit_test_framework ${HDF5_LINK_LIBRARIES})

# Link Eigen to the test executables
target_include_directories(test_computeStiffnessLoad PRIVATE ${EIGEN3_INCLUDE_DIR})
target_include_directories(test_convergence_analysis PRIVATE ${EIGEN3_INCLUDE_DIR})
target_include_directories(test_main PRIVATE ${EIGEN3_INCLUDE_DIR})

# Specify the directory to store test results
set(CTEST_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/test")

# Add test to CTest
enable_testing()
add_test(NAME ComputeStiffnessLoadTests COMMAND test_computeStiffnessLoad)
add_test(NAME ConvergenceAnalysisTests COMMAND test_convergence_analysis)
add_test(NAME MeshSolverTests COMMAND test_main)

# Set the path to lcov binaries (bin directory)
set(LCOV_BIN_PATH "${CMAKE_SOURCE_DIR}/m4/lcov-1.16/bin")
set(ENV{PATH} "$ENV{PATH}:${LCOV_BIN_PATH}")
set(COVERAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/coverage_report")
file(MAKE_DIRECTORY ${COVERAGE_OUTPUT_DIR})

# Define the coverage targets and steps
add_custom_target(coverage
    COMMAND ${CMAKE_COMMAND} -E echo "Running tests for coverage..."
    COMMAND ${CMAKE_BINARY_DIR}/test_computeStiffnessLoad
    COMMAND ${CMAKE_BINARY_DIR}/test_convergence_analysis
    COMMAND ${CMAKE_BINARY_DIR}/test_main
    COMMAND ${CMAKE_COMMAND} -E echo "Creating coverage report directory..."
    COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E echo "Collecting coverage data..."
    COMMAND ${LCOV_BIN_PATH}/lcov --capture --directory ${CMAKE_BINARY_DIR} --output-file ${COVERAGE_OUTPUT_DIR}/coverage.info
    COMMAND ${CMAKE_COMMAND} -E echo "Removing non-test files from coverage data..."
    # Remove non-test files by matching `test_*` files
    COMMAND ${LCOV_BIN_PATH}/lcov -r ${COVERAGE_OUTPUT_DIR}/coverage.info "*test_*" -o ${COVERAGE_OUTPUT_DIR}/filtered_coverage.info
    COMMAND ${CMAKE_COMMAND} -E echo "Generating HTML report..."
    COMMAND ${LCOV_BIN_PATH}/genhtml ${COVERAGE_OUTPUT_DIR}/filtered_coverage.info --output-directory ${COVERAGE_OUTPUT_DIR}
    COMMENT "Running code coverage analysis and generating HTML report"
)

# Cleaning rule for coverage
add_custom_target(clean_coverage
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${COVERAGE_OUTPUT_DIR}
    COMMENT "Cleaning up coverage reports"
)

# Allow 'make coverage' to clean and then regenerate
add_dependencies(coverage clean_coverage)
add_dependencies(coverage test_computeStiffnessLoad test_convergence_analysis test_main)

