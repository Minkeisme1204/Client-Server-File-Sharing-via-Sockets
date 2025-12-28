# Client-Server File Sharing via Sockets

A professional client-server file sharing application built using C++ socket programming. Features include binary file transfer, multi-client support, real-time metrics, and graceful error handling. Available as both CLI terminal applications and Qt-based GUI applications.

## ✨ Features

- 🔄 **File Transfer**: Upload (PUT), Download (GET), List files (LIST)
- 💾 **Binary Support**: Transfer any file type (text, audio, video, images, executables)
- 👥 **Multi-Client**: Handle up to 10 concurrent connections
- 📊 **Metrics Tracking**: Real-time statistics and CSV export
- 🎨 **Professional UI**: Interactive terminal interface AND graphical Qt5/Qt6 GUI
- 🛡️ **Robust**: Graceful shutdown, exception handling, data integrity
- 🔧 **Configurable**: Timeout, shared directory, verbose logging

## 🚀 Quick Start

### Option 1: CLI Terminal Applications

#### Build
```bash
cd build
cmake ..
make
```

#### Run Server
```bash
./build/server_test          # Default: port 8080, dir ./shared
./build/server_test 9000     # Custom port
./build/server_test 9000 /path/to/files  # Custom port & directory
```

#### Run Client
```bash
./build/client_test 127.0.0.1 8080  # Auto-connect
./build/client_test                  # Manual connect
```

#### Usage
```bash
# In client terminal:
Client> list                    # List files on server
Client> get test_file.txt       # Download file
Client> put myfile.mp3          # Upload file
Client> metrics                 # View statistics
Client> help                    # Show all commands
```

### Option 2: Qt GUI Applications

#### Build GUI
```bash
cd app
mkdir build && cd build
cmake ..
make
```

#### Run GUI
```bash
# Quick launch both server and client GUI:
cd app
./run_gui.sh

# Or run individually:
./app/build/server_gui    # Start server GUI
./app/build/client_gui    # Start client GUI
```

**GUI Features:**
- Server GUI: Start/stop server, view active clients, monitor metrics in real-time
- Client GUI: Connect to server, browse files, upload/download with progress, view transfer log
- Color-coded status indicators (green = connected/running, red = disconnected/stopped)
- Tabbed interface for better organization
- Export metrics to CSV

See [app/README.md](app/README.md) for detailed GUI documentation.

## 📖 Documentation

- [app/README.md](app/README.md) - Qt GUI applications guide
- [TEST_USAGE.md](TEST_USAGE.md) - Complete CLI usage guide
- [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) - Implementation details
- [ARCHITECTURE.md](ARCHITECTURE.md) - System architecture
- [BUILD.md](BUILD.md) - Build instructions

## 🎯 Test Applications

Fully-featured terminal applications for demonstration:
- **server_test.cpp**: Server with interactive commands, metrics, multi-threading
- **client_test.cpp**: Client with file transfer, auto-connect, metrics tracking

## 🧪 Automated Tests

```bash
./test_list.sh      # Test LIST command
./test_put.sh       # Test PUT command  
./test_get.sh       # Test GET command
./test_binary.sh    # Test binary file integrity
./test_shutdown.sh  # Test graceful shutdown
```

## 📁 Project Structure

```
├── include/          # Header files
│   ├── client.h
│   ├── server.h
│   └── core/        # Protocol & socket implementations
├── src/             # Source files
│   ├── client.cpp
│   ├── server.cpp
│   └── core/        # Core implementations
├── tests/           # Test applications
│   ├── client_test.cpp
│   └── server_test.cpp
├── build/           # Build output
└── shared/          # Default file sharing directory
```

## 🔧 Requirements

- **C++17** or later
- **CMake** 3.16+
- **Linux/macOS** (POSIX sockets)
- Standard libraries: pthread, filesystem

## 📝 License

See [LICENSE](LICENSE) file for details.
