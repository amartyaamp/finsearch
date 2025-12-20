## Time Series Pattern Matcher (FAISS Implementation)

This project uses C++ and the FAISS library to perform similarity search on Z-score normalized time series data. This document provides instructions for setting up the environment using Docker and building the executable.

### Prerequisites

You must have the following tools installed on your system:

Docker: Used to create a reproducible build and runtime environment.

Make: Used to orchestrate the build process (assumes a standard Makefile is present).

### 1. Building the Docker Image

The Docker image encapsulates the necessary dependencies (GCC, FAISS, etc.) required to compile and run the application.

Navigate to the root directory of the project where your Dockerfile is located.

Build the image and tag it as pattern-matcher:

```
docker build -t pattern-matcher .
```

### 2. Compiling the Code

We will use the custom Docker container to execute the build commands and compile the C++ source files (`engine.cpp`, `main.cpp`, `csv_parser.cpp`, etc.).

Run the Container in Build Mode: Start a container from the image and mount your local source directory `($(pwd)` to the `/app` directory inside the container. This allows the compiled binary to persist on your host machine.

```
docker run --rm -v "$(pwd)":/app pattern-matcher /bin/bash -c "make all"
```

`--rm`: Removes the container once it exits.

`-v "$(pwd)":/app`: Mounts the current directory to /app.

`/bin/bash -c "make all"`: Executes the make all command inside the container to compile the project.

Verify the Build: If successful, an executable file (e.g., pattern_match or main) will be created in your local project directory.

### 3. Running the Application

The application requires the historical price data file, which must be made accessible to the container at runtime.

Data Requirement: Ensure the `nifty_50.csv` file (or your input data) is available in a location that can be mounted to the container. The application is hardcoded to look for the data at `/app/data/nifty_50.csv`.

Execute the Binary: Run the application, ensuring the data directory is mounted correctly.
```
docker run --rm \
  -v "$(pwd)/data":/app/data \
  pattern-matcher \
  /app/pattern_match

```
The output will show the index creation and the nearest neighbor results based on the last 60 minutes of data.
