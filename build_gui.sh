#!/bin/bash

# Build GUI Applications
# Script to compile Qt5 GUI applications for File Transfer

echo "======================================"
echo "Building GUI Applications"
echo "======================================"

# Check if Qt5 is installed
if ! command -v qmake &> /dev/null; then
    echo "Warning: Qt5 (qmake) not found in PATH"
    echo "Please install Qt5 first:"
    echo "  Ubuntu/Debian: sudo apt-get install qt5-default qtbase5-dev"
    echo "  Fedora/CentOS: sudo dnf install qt5-qtbase-devel"
    echo "  macOS: brew install qt@5"
    echo ""
    read -p "Continue anyway? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Navigate to build directory
if [ ! -d "build" ]; then
    echo "Creating build directory..."
    mkdir build
fi

cd build

# Run CMake
echo ""
echo "Running CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build GUI applications
echo ""
echo "Building client_gui..."
make client_gui -j$(nproc)

echo ""
echo "Building server_gui..."
make server_gui -j$(nproc)

# Check if build was successful
if [ -f "client_gui" ] && [ -f "server_gui" ]; then
    echo ""
    echo "======================================"
    echo "✓ Build successful!"
    echo "======================================"
    echo ""
    echo "Executables:"
    echo "  Client GUI: ./build/client_gui"
    echo "  Server GUI: ./build/server_gui"
    echo ""
    echo "To run:"
    echo "  cd build"
    echo "  ./server_gui    # Start server first"
    echo "  ./client_gui    # Then start client"
    echo ""
else
    echo ""
    echo "======================================"
    echo "✗ Build failed!"
    echo "======================================"
    echo "Please check the error messages above."
    exit 1
fi
