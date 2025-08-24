#!/bin/bash

echo "Building Marine Generator Simulator Engine..."
echo

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake is not installed or not in PATH"
    echo "Please install CMake from https://cmake.org/download/"
    exit 1
fi

# Check if make is available
if ! command -v make &> /dev/null; then
    echo "ERROR: Make is not installed or not in PATH"
    echo "Please install make (build-essential on Ubuntu/Debian)"
    exit 1
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with CMake
echo "Configuring project..."
cmake ..
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

# Build the project
echo "Building project..."
make
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

echo
echo "Build completed successfully!"
echo "Executable location: build/generator-simulator"
echo
echo "To run the engine:"
echo "  cd build"
echo "  ./generator-simulator"
echo
