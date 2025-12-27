# File Transfer Client - Design Document

## 📋 Overview

A modern, well-structured C++ client library for file transfer operations using socket programming. The design follows object-oriented principles with a clear separation of concerns and uses an interface-based architecture.

## 🏗️ Architecture

### Design Pattern: Interface Segregation & Dependency Injection

```
┌──────────────────────────────────────────────┐
│            IClient (Interface)               │
│  - Defines contract for all client ops      │
└─────────────────┬────────────────────────────┘
                  │ implements
                  ▼
┌──────────────────────────────────────────────┐
│              Client (Concrete)               │
│  - Implements IClient interface              │
│  - Orchestrates all operations               │
└──────┬───────────┬───────────┬───────────────┘
       │           │           │
       │           │           │
       ▼           ▼           ▼
┌──────────┐ ┌─────────┐ ┌─────────────┐
│  Socket  │ │Protocol │ │   Metrics   │
│  Layer   │ │  Layer  │ │  Collector  │
└──────────┘ └─────────┘ └─────────────┘
```

## 📦 Components

### 1. IClient Interface (`IClient.h`)

**Purpose**: Define the contract for all client implementations

**Key Methods**:
- Connection: `connect()`, `disconnect()`, `isConnected()`
- File Operations: `uploadFile()`, `downloadFile()`, `listFiles()`, `deleteFile()`
- Metrics: `getTransferMetrics()`, `getConnectionMetrics()`
- Callbacks: `setProgressCallback()`, `setErrorCallback()`

**Benefits**:
- Allows multiple implementations (mock, test, production)
- Enables dependency injection for testing
- Clear contract for client behavior
- Supports polymorphism

### 2. Client Implementation (`Client.h` / `Client.cpp`)

**Purpose**: Concrete implementation of IClient

**Components**:
```cpp
class Client : public IClient {
private:
    std::unique_ptr<ClientSocket> socket_;
    std::unique_ptr<ClientProtocol> protocol_;
    std::unique_ptr<MetricsCollector> metricsCollector_;
    std::unique_ptr<ClientMetrics> coreMetrics_;
    // ... configuration and state
};
```

**Key Features**:
- ✅ Chunked file transfer with configurable chunk size
- ✅ Progress tracking with callbacks
- ✅ Comprehensive error handling
- ✅ Connection state management
- ✅ Metrics collection (transfer & connection)
- ✅ Verbose logging option

### 3. Core Components

#### ClientSocket (`core/client_socket.h`)
```cpp
class ClientSocket {
public:
    bool connectToServer(const std::string& ip, uint16_t port);
    void disconnect();
    ssize_t sendData(const uint8_t* data, size_t size);
    ssize_t receiveData(uint8_t* buffer, size_t size);
    bool isConnected() const;
};
```

**Responsibilities**:
- Low-level socket operations
- TCP connection management
- Data transmission

#### ClientProtocol (`core/client_protocol.h`)
```cpp
class ClientProtocol {
public:
    void request_list();
    void request_get(const std::string& filename, const std::string& save_dir);
    void request_put(const std::string& filepath);
};
```

**Responsibilities**:
- Protocol-level operations
- Message formatting
- Request handling

#### MetricsCollector (`metrics.h`)
```cpp
class MetricsCollector {
public:
    void startTransfer(uint64_t totalSize, uint32_t totalChunks);
    void updateTransfer(uint64_t bytesTransferred);
    TransferMetrics getTransferMetrics() const;
    ConnectionMetrics getConnectionMetrics() const;
    std::string getSummary() const;
};
```

**Responsibilities**:
- Real-time metrics tracking
- Transfer statistics
- Connection statistics
- Formatted output

## 🔄 Data Flow

### Upload Operation
```
User → Client::uploadFile()
  ↓
Check connection & file existence
  ↓
Start metrics tracking
  ↓
Protocol::request_put()
  ↓
Client::sendFileChunked()
  ↓
Loop: Read chunk → Socket::sendData() → Update metrics → Callback
  ↓
Complete metrics
  ↓
Return success/failure
```

### Download Operation
```
User → Client::downloadFile()
  ↓
Check connection
  ↓
Start metrics tracking
  ↓
Protocol::request_get()
  ↓
Client::receiveFileChunked()
  ↓
Loop: Socket::receiveData() → Write chunk → Update metrics → Callback
  ↓
Complete metrics
  ↓
Return success/failure
```

## 📊 Metrics System

### Transfer Metrics
- `totalBytes`: Total bytes transferred
- `bytesPerSecond`: Current transfer speed
- `averageSpeed`: Average transfer speed
- `progress`: Transfer progress (0.0-1.0)
- `elapsedTimeMs`: Time elapsed
- `estimatedTimeMs`: Estimated time remaining
- `chunksTransferred`: Number of chunks sent/received
- `totalChunks`: Total number of chunks
- `retries`: Number of retry attempts
- `errors`: Number of errors encountered

### Connection Metrics
- `packetsSent`: Total packets sent
- `packetsReceived`: Total packets received
- `bytesSent`: Total bytes sent
- `bytesReceived`: Total bytes received
- `roundTripTimeMs`: Round-trip time
- `connectionTimeMs`: Total connection time
- `isConnected`: Current connection status

## 🎯 Key Design Decisions

### 1. Interface-Based Design
**Rationale**: Allows for easy testing with mock implementations and supports multiple client types

### 2. Unique Pointers for Components
**Rationale**: Clear ownership semantics, automatic cleanup, no manual memory management

### 3. Chunked Transfer
**Rationale**: 
- Enables progress tracking
- Reduces memory usage for large files
- Allows resumable transfers (future enhancement)
- Better error recovery

### 4. Callback System
**Rationale**:
- Decouples UI from business logic
- Enables flexible notification mechanisms
- Supports real-time updates

### 5. Separate Core Components
**Rationale**:
- Single Responsibility Principle
- Easier to test individual components
- Can be reused in other contexts

## 🔧 Configuration

### Client Configuration Options
```cpp
// Chunk size for file transfers
client->setChunkSize(64 * 1024);  // 64KB default

// Enable verbose logging
client->setVerbose(true);

// Set callbacks
client->setProgressCallback([](double progress, uint64_t bytes) {
    // Handle progress updates
});

client->setErrorCallback([](const std::string& error) {
    // Handle errors
});
```

## 🧪 Testing Strategy

### Unit Tests
- Test each component in isolation
- Mock dependencies using IClient interface
- Test error conditions

### Integration Tests
- Test client with mock server
- Test file transfer operations
- Test metrics accuracy

### Example Mock:
```cpp
class MockClient : public IClient {
    // Implement interface for testing
};
```

## 📈 Performance Considerations

### Chunk Size Selection
- **Small files (<1MB)**: 32-64KB chunks
- **Medium files (1-100MB)**: 64-256KB chunks
- **Large files (>100MB)**: 256KB-1MB chunks

### Memory Usage
- Configurable chunk size controls memory footprint
- Streaming approach prevents loading entire file
- Typical memory usage: ~2x chunk size

### Network Optimization
- Reuse socket connection for multiple operations
- Batch small files when possible
- Consider compression for text files (future)

## 🔐 Security Considerations

### Current Implementation
- No encryption (plain TCP)
- No authentication

### Future Enhancements
- TLS/SSL encryption
- User authentication
- File integrity verification (checksums)
- Access control lists

## 🚀 Usage Examples

### Basic Usage
```cpp
#include "FilesTransfer/Client/Client.h"

using namespace FileTransfer;

// Create client
auto client = createClient();

// Connect
client->connect("127.0.0.1", 8080);

// Upload
client->uploadFile("local_file.txt");

// Download
client->downloadFile("remote_file.txt", ".");

// Disconnect
client->disconnect();
```

### With Progress Tracking
```cpp
client->setProgressCallback([](double progress, uint64_t bytes) {
    std::cout << "Progress: " << (progress * 100) << "%" << std::endl;
});

client->uploadFile("large_file.bin");
```

### Error Handling
```cpp
client->setErrorCallback([](const std::string& error) {
    std::cerr << "Error: " << error << std::endl;
});

if (!client->uploadFile("file.txt")) {
    std::cerr << "Upload failed: " << client->getLastError() << std::endl;
}
```

## 📚 Dependencies

### Required
- C++17 compiler (g++ 7+, clang++ 5+)
- POSIX socket API
- pthread library

### Optional
- CMake 3.10+ (for CMake build)
- OpenSSL (for future encryption support)

## 🔮 Future Enhancements

1. **Resume Support**: Continue interrupted transfers
2. **Compression**: Compress data before transfer
3. **Encryption**: Add TLS/SSL support
4. **Authentication**: Username/password or token-based
5. **Batch Operations**: Transfer multiple files efficiently
6. **Bandwidth Throttling**: Limit transfer speed
7. **Progress Persistence**: Save/restore transfer state
8. **Concurrent Transfers**: Multiple simultaneous transfers
9. **File Synchronization**: Sync directories between client/server
10. **HTTP API**: REST API alternative to socket protocol

## 📝 API Reference

See individual header files for detailed API documentation:
- [IClient.h](../include%20/FilesTransfer/Client/IClient.h) - Interface definition
- [Client.h](../include%20/FilesTransfer/Client/Client.h) - Client implementation
- [metrics.h](../include%20/FilesTransfer/Client/metrics.h) - Metrics system

## 👥 Contributing

Contributions are welcome! Please follow these guidelines:
1. Maintain the interface-based design
2. Add unit tests for new features
3. Update documentation
4. Follow existing code style
5. Create pull requests with clear descriptions

## 📄 License

See LICENSE file in project root.
