# 🔨 Build Guide for File Transfer System

Complete build guide for the client-server file transfer application.

## Table of Contents
- [Prerequisites](#prerequisites)
- [Quick Build](#quick-build)
- [Detailed Build Steps](#detailed-build-steps)
- [Build Targets](#build-targets)
- [Platform-Specific Instructions](#platform-specific-instructions)
- [Troubleshooting](#troubleshooting)

## Prerequisites

### Required Tools

| Tool | Minimum Version | Purpose |
|------|----------------|---------|
| CMake | 3.16+ | Build system generator |
| C++ Compiler | C++17 support | GCC 7+, Clang 5+, MSVC 2017+ |
| Make/Ninja | Any recent | Build tool |

### Required Libraries

| Library | Purpose | Notes |
|---------|---------|-------|
| pthread | Threading support | System default on Linux/macOS |
| std::filesystem | File operations | Built into C++17 |

### Installation Commands

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install cmake g++ make
```

**Fedora/RedHat:**
```bash
sudo dnf install cmake gcc-c++ make
```

**macOS:**
```bash
brew install cmake
xcode-select --install  # Includes clang
```

## Quick Build

### Standard Build (Recommended)

```bash
cd Client-Server-File-Sharing-via-Sockets
cd build
cmake ..
make
```

**Output:**
- `libfiletransfer.a` - Core library
- `server_test` - Server application
- `client_test` - Client application

### One-Line Build

```bash
cmake -B build && cmake --build build -j$(nproc)
```

## Detailed Build Steps

### Step 1: Configure

```bash
cd build
cmake ..
```

**What this does:**
- Detects C++17 compiler
- Scans for source files (src/**/*.cpp)
- Configures include paths
- Generates Makefiles

**Expected Output:**
```
-- The CXX compiler identification is GNU 11.4.0
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - works
-- Configuring done
-- Generating done
-- Build files have been written to: .../build
```

### Step 2: Build

```bash
make
```

**Build order:**
1. Compile core library sources (src/core/**/*.cpp)
2. Link into libfiletransfer.a (static library)
3. Compile test applications (tests/*.cpp)
4. Link executables with library

**Expected Output:**
```
[  7%] Building CXX object CMakeFiles/filetransfer.dir/src/client.cpp.o
[ 14%] Building CXX object CMakeFiles/filetransfer.dir/src/core/Client/client_metrics.cpp.o
...
[ 71%] Linking CXX static library libfiletransfer.a
[ 78%] Building CXX object CMakeFiles/server_test.dir/tests/server_test.cpp.o
[ 85%] Linking CXX executable server_test
[ 92%] Building CXX object CMakeFiles/client_test.dir/tests/client_test.cpp.o
[100%] Linking CXX executable client_test
[100%] Built target client_test
```

### Step 3: Verify Build

```bash
ls -lh build/
```

**Should see:**
```
-rw-r--r-- libfiletransfer.a  (static library ~200KB)
-rwxr-xr-x server_test        (executable ~180KB)
-rwxr-xr-x client_test        (executable ~110KB)
```

### Step 4: Test

```bash
# From project root
./build/server_test &
./build/client_test 127.0.0.1 8080
```

## Build Targets

### All Targets

```bash
make              # Build everything
make filetransfer # Build library only
make server_test  # Build server only
make client_test  # Build client only
make clean        # Remove build artifacts
```

### Parallel Build

```bash
make -j$(nproc)   # Use all CPU cores
make -j4          # Use 4 cores
```

## Build Configurations

### Debug Build

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

**Features:**
- Debug symbols (-g)
- No optimization (-O0)
- Assertions enabled
- Larger binaries

### Release Build (Recommended for Production)

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

**Features:**
- Full optimization (-O3)
- No debug symbols
- Smaller, faster binaries
- NDEBUG defined

### Release with Debug Info

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make
```

## Platform-Specific Instructions

### Linux

**Standard build works:**
```bash
cd build && cmake .. && make
```

**Install system-wide (optional):**
```bash
sudo make install
```

### macOS

**Using Homebrew compiler:**
```bash
export CC=/usr/local/bin/gcc-13
export CXX=/usr/local/bin/g++-13
cd build && cmake .. && make
```

**Using Apple Clang:**
```bash
cd build && cmake .. && make
# Works out of the box
```

### Windows (WSL Recommended)

**Using WSL Ubuntu:**
```bash
# Same as Linux
cd build && cmake .. && make
```

**Native Windows (Advanced):**
```bash
# Use Visual Studio 2019+
cmake -B build -G "Visual Studio 16 2019"
cmake --build build --config Release
```

## Compiler Options

### View Compiler Flags

```bash
cd build
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
make
```

### Custom Compiler Flags

```bash
cmake -DCMAKE_CXX_FLAGS="-Wall -Wextra -O2" ..
```

## Troubleshooting

### Problem: CMake not found
```bash
# Ubuntu/Debian
sudo apt-get install cmake

# macOS
brew install cmake
```

### Problem: C++17 not supported
```bash
# Update compiler
sudo apt-get install g++-9  # Ubuntu
```

### Problem: pthread not found
```bash
# Usually auto-detected, but force if needed:
cmake -DCMAKE_THREAD_PREFER_PTHREAD=TRUE ..
```

### Problem: Build errors in client_test.cpp
```bash
# Clean and rebuild
cd build
rm -rf *
cmake ..
make
```

### Problem: "No such file or directory" for includes
```bash
# Check include paths
cmake --build build --verbose
# Ensure project structure is intact
ls include/core/Client/
ls include/core/Server/
```

### Problem: Linker errors
```bash
# Rebuild library
cd build
make clean
make filetransfer
make server_test
make client_test
```

## Advanced Build Options

### Cross-Compilation

```bash
cmake -DCMAKE_TOOLCHAIN_FILE=path/to/toolchain.cmake ..
```

### Static Linking (All libs)

```bash
cmake -DCMAKE_EXE_LINKER_FLAGS="-static" ..
```

### Custom Install Prefix

```bash
cmake -DCMAKE_INSTALL_PREFIX=/opt/filetransfer ..
make
make install
```

## Build Performance

**Typical build times:**
- Clean build: 10-30 seconds
- Incremental: 1-5 seconds
- Parallel (-j8): 5-10 seconds

## Verification

### Check Binary Type

```bash
file build/server_test
# Output: ELF 64-bit LSB executable, dynamically linked

ldd build/server_test
# Should show: libpthread, libstdc++, libc
```

### Check Symbols

```bash
nm build/libfiletransfer.a | grep " T "
# Shows exported symbols
```

### Test Executables

```bash
./build/server_test --help 2>/dev/null || echo "Server binary OK"
./build/client_test --help 2>/dev/null || echo "Client binary OK"
```

## Clean Build

```bash
cd build
make clean          # Remove object files
rm -rf *           # Complete clean
cmake .. && make   # Rebuild from scratch
-- Generating done
-- Build files written to: build/
```

### Step 2: Build

Compile the source code into a shared library:

```bash
cmake --build build
```

Or with parallel jobs:
```bash
cmake --build build -j$(nproc)  # Linux/macOS
cmake --build build -j8          # Windows (specify core count)
```

**What this does:**
- Compiles all `.cpp` files to object files
- Links object files into `libfiletransfer.so`
- Creates version symlinks
- Places output in `build/lib/`

**Output:**
```
[ 20%] Building CXX object CMakeFiles/FileTransferCore.dir/src/core/Client.cpp.o
[ 40%] Building CXX object CMakeFiles/FileTransferCore.dir/src/core/Client/client_socket.cpp.o
[ 60%] Building CXX object CMakeFiles/FileTransferCore.dir/src/core/Client/client_protocol.cpp.o
[ 80%] Building CXX object CMakeFiles/FileTransferCore.dir/src/core/Client/client_metrics.cpp.o
[100%] Linking CXX shared library lib/libfiletransfer.so
```

### Step 3: Verify Build

Check that the library was built:

```bash
# Check library exists
ls -lh build/lib/libfiletransfer.so*

# Check library info
file build/lib/libfiletransfer.so.1.0.0
ldd build/lib/libfiletransfer.so.1.0.0  # Check dependencies
```

### Step 4: Install (Optional)

Install the library system-wide:

```bash
sudo cmake --build build --target install
```

**Default installation locations:**
```
/usr/local/lib/libfiletransfer.so*
/usr/local/include/core/
/usr/local/lib/cmake/FileTransferCore/
```

To change install prefix:
```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/opt/filetransfer
sudo cmake --build build --target install
```

## Build Options

### CMake Build Options

```cmake
# Build type
-DCMAKE_BUILD_TYPE=Debug          # Debug symbols, no optimization
-DCMAKE_BUILD_TYPE=Release        # Optimized, no debug symbols
-DCMAKE_BUILD_TYPE=RelWithDebInfo # Optimized with debug symbols
-DCMAKE_BUILD_TYPE=MinSizeRel     # Optimized for size

# Library type
-DBUILD_SHARED_LIBS=ON            # Build shared library (default)
-DBUILD_SHARED_LIBS=OFF           # Build static library

# Project options
-DBUILD_EXAMPLES=ON               # Build example applications
-DBUILD_TESTS=ON                  # Build test suite
-DENABLE_STRICT_WARNINGS=ON       # Enable strict warnings (default)

# Installation
-DCMAKE_INSTALL_PREFIX=/path      # Install location
```

### Example Configurations

**Development Build:**
```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -DENABLE_STRICT_WARNINGS=ON
```

**Production Build:**
```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_INSTALL_PREFIX=/usr/local
```

**Minimal Build:**
```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DENABLE_STRICT_WARNINGS=OFF
```

## Build Configurations

### Debug Build

For development and debugging:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

**Characteristics:**
- Debug symbols included (`-g`)
- No optimization (`-O0`)
- Assertions enabled
- Larger binary size
- Slower execution

**Use when:**
- Developing new features
- Debugging issues
- Running under debugger (gdb, lldb)

### Release Build

For production deployment:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**Characteristics:**
- Full optimization (`-O3`)
- No debug symbols
- Assertions disabled
- Smaller binary size
- Fastest execution

**Use when:**
- Deploying to production
- Performance testing
- Creating distribution packages

### RelWithDebInfo Build

Optimized with debug info:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
```

**Characteristics:**
- Optimization enabled (`-O2`)
- Debug symbols included
- Good balance

**Use when:**
- Performance testing with profiling
- Production debugging
- Release candidate testing

## Platform-Specific Instructions

### Linux

```bash
# Install dependencies
sudo apt-get install cmake g++ make

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

# Install
sudo cmake --build build --target install
sudo ldconfig  # Update library cache
```

**Library location:** `/usr/local/lib/libfiletransfer.so`

### macOS

```bash
# Install dependencies
brew install cmake

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(sysctl -n hw.ncpu)

# Install
sudo cmake --build build --target install
```

**Library location:** `/usr/local/lib/libfiletransfer.dylib`

### Windows

**Using Visual Studio:**
```powershell
# Configure for Visual Studio
cmake -B build -G "Visual Studio 17 2022" -A x64

# Build
cmake --build build --config Release

# Install (as Administrator)
cmake --build build --target install --config Release
```

**Using MinGW:**
```bash
# Configure for MinGW
cmake -B build -G "MinGW Makefiles"

# Build
cmake --build build

# Install
cmake --build build --target install
```

## Advanced Build Options

### Using Ninja (Faster Builds)

```bash
# Install Ninja
sudo apt-get install ninja-build  # Ubuntu
brew install ninja                # macOS

# Configure with Ninja
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# Build (automatic parallelism)
cmake --build build
```

### Cross-Compilation

```bash
# Create toolchain file: toolchain-arm.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# Build with toolchain
cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-arm.cmake
cmake --build build
```

### Static Analysis

**Using clang-tidy:**
```bash
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
clang-tidy src/**/*.cpp -p build
```

**Using cppcheck:**
```bash
cppcheck --enable=all --project=build/compile_commands.json
```

## Incremental Builds

After making code changes:

```bash
# Just rebuild (CMake detects changes)
cmake --build build

# If you added/removed files
cmake -B build  # Re-scan
cmake --build build
```

## Clean Builds

### Clean object files (keep configuration):
```bash
cmake --build build --target clean
```

### Complete clean (remove build directory):
```bash
rm -rf build
cmake -B build
cmake --build build
```

## Troubleshooting

### Common Issues

#### Issue: CMake not found
```bash
cmake: command not found
```
**Solution:**
```bash
# Ubuntu/Debian
sudo apt-get install cmake

# macOS
brew install cmake

# Or install from source
pip3 install cmake
```

#### Issue: C++17 not supported
```
error: #error This file requires compiler and library support for the ISO C++ 2017 standard.
```
**Solution:**
```bash
# Upgrade compiler
sudo apt-get install g++-9  # GCC 9+
```

#### Issue: filesystem not found
```
error: 'filesystem' is not a member of 'std'
```
**Solution:**
Add `-lstdc++fs` or upgrade to GCC 9+. CMake handles this automatically.

#### Issue: pthread not found
```
undefined reference to `pthread_create'
```
**Solution:**
```bash
sudo apt-get install libpthread-stubs0-dev
```

#### Issue: Permission denied during install
```bash
# Use sudo
sudo cmake --build build --target install
```

### Build Verification

After successful build:

```bash
# Check library
ls -lh build/lib/libfiletransfer.so*

# Check symbols
nm -D build/lib/libfiletransfer.so.1.0.0 | grep Client

# Check dependencies
ldd build/lib/libfiletransfer.so.1.0.0

# Run tests (if enabled)
cd build && ctest
```

### Getting Help

If you encounter issues:

1. Check CMake output for errors
2. Verify all dependencies are installed
3. Try a clean build (`rm -rf build`)
4. Check compiler version
5. Review [ARCHITECTURE.md](ARCHITECTURE.md)
6. Create an issue on GitHub

## Build Performance Tips

### Parallel Compilation

```bash
# Use all CPU cores
cmake --build build -j$(nproc)

# Limit to 4 cores
cmake --build build -j4
```

### Compiler Cache (ccache)

```bash
# Install ccache
sudo apt-get install ccache

# Configure CMake to use ccache
cmake -B build -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

### Precompiled Headers

For faster rebuilds, CMake can use precompiled headers automatically in C++20 mode.

## CI/CD Integration

### GitHub Actions

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: sudo apt-get install cmake g++
      - name: Configure
        run: cmake -B build -DCMAKE_BUILD_TYPE=Release
      - name: Build
        run: cmake --build build -j$(nproc)
      - name: Test
        run: cd build && ctest
```

### GitLab CI

```yaml
build:
  image: gcc:latest
  script:
    - apt-get update && apt-get install -y cmake
    - cmake -B build -DCMAKE_BUILD_TYPE=Release
    - cmake --build build -j$(nproc)
    - cd build && ctest
```

## Summary

**Recommended build command:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)
```

**For development:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build -j$(nproc)
cd build && ctest
```

**For installation:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build -j$(nproc)
sudo cmake --build build --target install
```

---

For more information, see:
- [ARCHITECTURE.md](ARCHITECTURE.md) - Library architecture
- [PACKAGE_OVERVIEW.md](PACKAGE_OVERVIEW.md) - Package contents
- [README.md](README.md) - Project overview
