# 🚀 File Transfer Client Library

A professional, well-architected C++ client library for file transfer operations using socket programming. Designed for the Computer Networks course at HUST.

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Build](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

## ✨ Features

### Core Capabilities
- ✅ **Upload/Download Files** - Efficient file transfer with progress tracking
- ✅ **List Files** - Browse available files on server
- ✅ **Delete Files** - Remove files from server
- ✅ **Connection Management** - Robust connection handling with ping/pong
- ✅ **Progress Tracking** - Real-time transfer progress with callbacks
- ✅ **Performance Metrics** - Comprehensive statistics (speed, RTT, throughput)
- ✅ **Error Handling** - Graceful error recovery and reporting
- ✅ **Chunked Transfer** - Configurable chunk size for optimal performance

### Architecture Highlights
- 🏗️ **Modular Design** - Clean separation of concerns with layered architecture
- 📦 **Modular Components** - Separated socket, protocol, and metrics layers
- 🎯 **SOLID Principles** - Clean, maintainable, testable code
- 🔧 **Configurable** - Chunk size, verbose mode, callbacks
- 🧪 **Testable** - Mock-friendly design with dependency injection
- 📊 **Metrics-Driven** - Built-in performance monitoring

## 📁 Project Structure

```
Client-Server-File-Sharing-via-Sockets/
├── include /FilesTransfer/Client/
│   ├── Client.h                    # Client interface
│   ├── Client.h                     # Client implementation header
│   ├── metrics.h                    # Metrics system
│   └── core/
│       ├── client_socket.h          # Socket layer
│       ├── client_protocol.h        # Protocol layer
│       └── client_metrics.h         # Core metrics
├── src/Client/
│   ├── Client.cpp                   # Main implementation
│   ├── metrics.cpp                  # Metrics implementation
│   ├── client_app.cpp              # Interactive application
│   └── core/                        # Core implementations
├── DESIGN.md                        # Design documentation
├── QUICKSTART.md                    # Quick start guide
├── CMakeLists_new.txt              # CMake configuration
└── Makefile_new                     # Makefile
```

## 🚀 Quick Start

### 1. Build the Project

```bash
# Using Makefile
make -f Makefile_new release

# Or using CMake
mkdir build && cd build
cmake -f ../CMakeLists_new.txt ..
cmake --build .
```

### 2. Run the Client Application

```bash
./build/bin/client_app
```

### 3. Use in Your Code

```cpp
#include "FilesTransfer/Client/Client.h"

using namespace FileTransfer;

int main() {
    // Create client
    auto client = createClient();
    
    // Setup callbacks
    client->setProgressCallback([](double progress, uint64_t bytes) {
        std::cout << "Progress: " << (progress * 100) << "%" << std::endl;
    });
    
    // Connect and transfer
    client->connect("127.0.0.1", 8080);
    client->uploadFile("myfile.txt");
    
    // Show statistics
    std::cout << client->getMetricsSummary() << std::endl;
    
    return 0;
}
```

## 📖 Documentation

| Document | Description |
|----------|-------------|
| [QUICKSTART.md](QUICKSTART.md) | Step-by-step guide for getting started |
| [DESIGN.md](DESIGN.md) | Detailed architecture and design decisions |
| [client.h](include/client.h) | Client API reference |
| [Client.h](include%20/FilesTransfer/Client/Client.h) | Implementation API reference |

## 🎯 Use Cases

### Interactive File Transfer
Run the built-in client application for interactive file operations:
```bash
./build/bin/client_app
```

### Programmatic Integration
Integrate the library into your own applications:
```cpp
auto client = createClient();
client->connect(serverIP, port);
client->uploadFile(filepath);
```

### Automated Testing
Use the interface for easy mocking and testing:
```cpp
class MockClient : public Client {
    // Implement for testing
};
```

## 🔧 Configuration

### Chunk Size
```cpp
client->setChunkSize(128 * 1024);  // 128KB chunks
```

### Callbacks
```cpp
// Progress updates
client->setProgressCallback([](double progress, uint64_t bytes) {
    displayProgress(progress, bytes);
});

// Error notifications
client->setErrorCallback([](const std::string& error) {
    logError(error);
});
```

### Verbose Logging
```cpp
client->setVerbose(true);
```

## 📊 Performance Metrics

The client tracks comprehensive metrics:

### Transfer Metrics
- Transfer speed (current and average)
- Progress percentage
- Elapsed time
- Estimated time remaining
- Chunk statistics
- Error counts

### Connection Metrics
- Packets sent/received
- Bytes sent/received
- Round-trip time (RTT)
- Connection duration
- Connection status

Access metrics:
```cpp
auto transferMetrics = client->getTransferMetrics();
auto connectionMetrics = client->getConnectionMetrics();
std::cout << client->getMetricsSummary();
```

## 🏗️ Architecture

```
┌─────────────────────────────────────────┐
│          Client (Interface)            │
│   - Defines contract                    │
└──────────────┬──────────────────────────┘
               │ implements
               ▼
┌─────────────────────────────────────────┐
│       Client (Implementation)           │
│   - Orchestrates operations             │
└──┬──────────┬──────────┬────────────────┘
   │          │          │
   ▼          ▼          ▼
┌────────┐ ┌────────┐ ┌──────────┐
│ Socket │ │Protocol│ │ Metrics  │
│ Layer  │ │ Layer  │ │Collector │
└────────┘ └────────┘ └──────────┘
```

## 🔒 Design Principles

1. **Interface Segregation** - Clean contract with Client
2. **Dependency Injection** - Components injected via constructors
3. **Single Responsibility** - Each class has one clear purpose
4. **Open/Closed** - Open for extension, closed for modification
5. **Separation of Concerns** - Socket, protocol, and business logic separated

## 🧪 Testing

The interface-based design enables easy testing:

```cpp
// Create mock for testing
class MockClient : public Client {
    bool connect(const std::string& ip, uint16_t port) override {
        // Mock implementation
        return true;
    }
    // ... other methods
};

// Use in tests
std::unique_ptr<Client> client = std::make_unique<MockClient>();
// Test your code with the mock
```

## 🛠️ Build Requirements

- **Compiler**: g++ 7+ or clang++ 5+ (C++17 support)
- **Build System**: Make or CMake 3.10+
- **Dependencies**: pthread
- **OS**: Linux, macOS, or Unix-like systems

## 📈 Performance Tips

1. **Adjust chunk size** based on file size and network:
   - Small files: 32-64KB
   - Medium files: 64-256KB  
   - Large files: 256KB-1MB

2. **Use callbacks** for responsive UI

3. **Monitor metrics** to identify bottlenecks

4. **Reuse connections** for multiple operations

## 🚧 Future Enhancements

- [ ] Resume interrupted transfers
- [ ] TLS/SSL encryption
- [ ] Authentication system
- [ ] Compression support
- [ ] Concurrent transfers
- [ ] Bandwidth throttling
- [ ] Directory synchronization
- [ ] REST API alternative

## 🤝 Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Follow the existing architecture
4. Add tests for new features
5. Update documentation
6. Submit a pull request

## 📝 Example Application

The project includes a full-featured interactive client:

```bash
./build/bin/client_app

╔══════════════════════════════════════╗
║   File Transfer Client Menu         ║
╠══════════════════════════════════════╣
║ 1. Connect to server                ║
║ 2. Upload file                      ║
║ 3. Download file                    ║
║ 4. List files                       ║
║ 5. Delete file                      ║
║ 6. Ping server                      ║
║ 7. Show metrics                     ║
║ 8. Toggle verbose mode              ║
║ 9. Disconnect                       ║
║ 0. Exit                             ║
╚══════════════════════════════════════╝
```

## 📞 Support

- 📖 Read the [Quick Start Guide](QUICKSTART.md)
- 🏗️ Check the [Design Document](DESIGN.md)
- 💻 Browse the [API Reference](include%20/FilesTransfer/Client/)
- 🐛 Report issues on GitHub

## 📄 License

See [LICENSE](LICENSE) file for details.

## 👥 Authors

Designed and developed for Computer Networks course at HUST.

---

**Built with ❤️ using modern C++ practices**
