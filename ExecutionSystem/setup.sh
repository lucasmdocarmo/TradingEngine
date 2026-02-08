#!/bin/bash
echo "Installing dependencies using Homebrew..."
brew install cmake boost openssl nlohmann-json

echo "Creating build directory..."
mkdir -p build
cd build

echo "Running CMake..."
cmake ..

echo "Building project..."
make

echo "Build complete. Run ./execution_engine to start."
