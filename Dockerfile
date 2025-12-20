FROM ubuntu:22.04

# 1. Install Build Tools & BLAS/LAPACK (FAISS Dependencies)
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    wget \
    git \
    libopenblas-dev \
    ca-certificates \
    libgtest-dev \
    # Compiles gtest static library from source installed by libgtest-dev
    && cd /usr/src/googletest/googletest && cmake . && make && cp lib/*.a /usr/lib \ 
    && rm -rf /var/lib/apt/lists/*

# --- NEW FIX: UPGRADE CMAKE TO 3.28 ---
# FAISS requires CMake >= 3.24, but Ubuntu 22.04 only has 3.22.1
WORKDIR /tmp
RUN wget -q https://github.com/Kitware/CMake/releases/download/v3.28.0/cmake-3.28.0-linux-aarch64.tar.gz && \
    tar -xzf cmake-3.28.0-linux-aarch64.tar.gz && \
    mv cmake-3.28.0-linux-aarch64/bin/* /usr/local/bin && \
    mv cmake-3.28.0-linux-aarch64/share/* /usr/local/share && \
    rm -rf cmake-3.28.0-linux-aarch64*
# We use the aarch64 binary to match your M4 Mac and the Ubuntu base image.
WORKDIR /

# 2. Install FAISS from Source (C++ Native)
WORKDIR /faiss_build
RUN git clone --depth 1 https://github.com/facebookresearch/faiss.git .
RUN cmake -DFAISS_ENABLE_GPU=OFF \
          -DFAISS_ENABLE_PYTHON=OFF \
          -DBUILD_SHARED_LIBS=ON \
          -DBUILD_TESTING=OFF \
          -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          . && \
    make -j$(nproc) && \
    make install

# 3. Final Setup
WORKDIR /app