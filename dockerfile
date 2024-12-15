# Base image
FROM ubuntu:20.04

# Install essential dependencies
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    g++ \
    cmake \
    wget \
    make \
    zlib1g-dev \
    libcurl4-openssl-dev \
    libhdf5-dev \
    pkg-config \
    python3 \
    python3-pip \
    valgrind \
    && rm -rf /var/lib/apt/lists/*

# Install Boost from source
RUN wget -q https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/boost_1_82_0.tar.gz && \
    tar -xzf boost_1_82_0.tar.gz && \
    cd boost_1_82_0 && \
    ./bootstrap.sh --prefix=/usr/local && \
    ./b2 install && \
    cd .. && rm -rf boost_1_82_0 boost_1_82_0.tar.gz

# Install HDF5 from source
RUN wget -q https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.12/hdf5-1.12.2/src/hdf5-1.12.2.tar.gz && \
    tar -xzf hdf5-1.12.2.tar.gz && \
    cd hdf5-1.12.2 && \
    ./configure --prefix=/usr/local/hdf5 --enable-cxx && \
    make -j$(nproc) && \
    make install && \
    cd .. && rm -rf hdf5-1.12.2 hdf5-1.12.2.tar.gz

# Add HDF5 to library path
ENV LD_LIBRARY_PATH="/usr/local/hdf5/lib:${LD_LIBRARY_PATH}"
ENV PKG_CONFIG_PATH="/usr/local/hdf5/lib/pkgconfig:${PKG_CONFIG_PATH}"

# Install Python libraries
RUN pip3 install numpy pandas matplotlib
RUN HDF5_DIR=/usr/local/hdf5 pip3 install --no-binary=h5py h5py

# Install Eigen
RUN wget -q https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz && \
    tar -xzf eigen-3.4.0.tar.gz && \
    cd eigen-3.4.0 && \
    mkdir build && cd build && \
    cmake .. && \
    make install && \
    cd ../.. && rm -rf eigen-3.4.0 eigen-3.4.0.tar.gz

# Environment variables
ENV HDF5_DIR=/usr/local/hdf5
ENV PATH="${HDF5_DIR}/bin:${PATH}"
ENV EIGEN3_INCLUDE_DIR=/usr/include/eigen3

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Build project
# RUN mkdir build && cd build && \
#     cmake -DEIGEN3_INCLUDE_DIR=${EIGEN3_INCLUDE_DIR} -DHDF5_ROOT=${HDF5_DIR} .. && \
#     make