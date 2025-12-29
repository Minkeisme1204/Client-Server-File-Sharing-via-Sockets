# Client-Server File Transfer System - Design Document

## 📋 Overview

A robust, multi-threaded C++ client-server system for file transfer operations using TCP socket programming. The design emphasizes thread safety, real-time metrics tracking, and dynamic configuration. The architecture supports concurrent client connections with isolated session handling and comprehensive performance monitoring.

## 🏗️ Architecture

### Design Pattern: Multi-threaded Client-Server with Layered Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                          CLIENT SIDE                            │
│                                                                 │
│  ┌──────────────────────────────────────────────────────┐      │
│  │              Client (Concrete Class)                 │      │
│  │         High-level file transfer API                 │      │
│  │      - Orchestrates all operations                   │      │
│  └──────┬───────────┬───────────┬──────────────────────┘      │
│         │           │           │                              │
│         ▼           ▼           ▼                              │
│  ┌──────────┐ ┌─────────┐ ┌─────────────┐                    │
│  │  Socket  │ │Protocol │ │   Metrics   │                    │
│  │  Layer   │ │  Layer  │ │  (atomic)   │                    │
│  └──────────┘ └─────────┘ └─────────────┘                    │
└─────────────────────────────────────────────────────────────────┘
                                │
                                │ TCP Connection
                                ▼
┌─────────────────────────────────────────────────────────────────┐
│                          SERVER SIDE                            │
│                                                                 │
│  ┌──────────────────────────────────────────────────────┐      │
│  │              Server (Main Thread)                    │      │
│  │    - Accept connections                              │      │
│  │    - Manage sessions                                 │      │
│  │    - Dynamic shared directory (shared_ptr)           │      │
│  └──────┬───────────────────────────────────────────────┘      │
│         │                                                       │
│         │ Spawns worker threads                                │
│         ▼                                                       │
│  ┌──────────────────────────────────────────────────────┐      │
│  │        ClientSession (Per-Client Thread)             │      │
│  │    - Isolated protocol handler                       │      │
│  │    - Shared directory pointer                        │      │
│  │    - Metrics pointer                                 │      │
│  └──────┬───────────┬───────────────────────────────────┘      │
│         │           │                                           │
│         ▼           ▼                                           │
│  ┌─────────┐ ┌─────────────┐                                  │
│  │Protocol │ │   Metrics   │                                  │
│  │  Layer  │ │  (thread-   │                                  │
│  │         │ │   safe)     │                                  │
│  └─────────┘ └─────────────┘                                  │
└─────────────────────────────────────────────────────────────────┘
```

## 📦 Components

### 1. Client Implementation (`Client.h` / `Client.cpp`)

**Purpose**: High-level client API for file transfer operations

**Key Methods**:
- Connection: `connect()`, `disconnect()`, `isConnected()`
- File Operations: `listFiles()`, `getFile()`, `putFile()`
- Metrics: `getMetrics()`, `displayMetrics()`, `exportMetrics()`
- Configuration: `setTimeout()`, `setVerbose()`

**Components**:
```cpp
class Client {
private:
    std::unique_ptr<ClientSocket> socket_;
    std::unique_ptr<ClientProtocol> protocol_;
    ClientMetrics metrics_;
    int timeout_;
    bool verbose_;
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

#### ClientSocket (`core/Client/client_socket.h`)
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
- Low-level TCP socket operations
- Connection management
- Blocking send/receive with proper error handling

#### ServerSocket (`core/Server/server_socket.h`)
```cpp
class ServerSocket {
public:
    bool bind(uint16_t port);
    int acceptConnection(std::string& clientAddr);
    static ssize_t sendData(int sockfd, const uint8_t* data, size_t size);
    static ssize_t receiveData(int sockfd, uint8_t* buffer, size_t size);
};
```

**Responsibilities**:
- Server socket creation and binding
- Accept incoming connections
- Static methods for session I/O

#### ClientProtocol (`core/Client/client_protocol.h`)
```cpp
class ClientProtocol {
public:
    bool request_list(std::vector<std::string>& files);
    bool request_get(const std::string& filename, const std::string& save_dir);
    bool request_put(const std::string& filepath);
    void setMetrics(ClientMetrics* metrics);
};
```

**Protocol Commands**:
- `CMD_LIST (0x01)`: Request file listing
- `CMD_GET (0x02)`: Download file from server
- `CMD_PUT (0x03)`: Upload file to server

**Responsibilities**:
- Binary protocol implementation
- Command serialization
- File transfer with chunking (8KB default)
- Metrics updates during transfer

#### ServerProtocol (`core/Server/server_protocol.h`)
```cpp
class ServerProtocol {
public:
    void processRequest(int clientFd);
    void setSharedDirectoryPtr(std::shared_ptr<std::string> dir);
    void setMetrics(ServerMetrics* metrics);
private:
    void handleListCommand(int clientFd);
    void handleGetCommand(int clientFd);
    void handlePutCommand(int clientFd);
};
```

**Responsibilities**:
- Process incoming client commands
- File operations (list, send, receive)
- Update metrics (bytes, files, latency)
- Use shared directory pointer

#### ClientMetrics (`core/Client/client_metrics.h`)
```cpp
struct ClientMetrics {
    double rtt_ms;  // Round-Trip Time (EMA)
    double throughput_kbps;  // Transfer throughput
    double packet_loss_rate;  // Calculated from failed/total
    double transfer_latency_ms;  // Last transfer time
    std::atomic<uint64_t> total_requests{0};
    std::atomic<uint64_t> failed_requests{0};
};
```

**Thread Safety**: Atomic counters for safe concurrent access

#### ServerMetrics (`core/Server/server_metrics.h`)
```cpp
class ServerMetrics {
private:
    std::atomic<uint64_t> totalConnections;
    std::atomic<uint64_t> activeConnections;
    std::atomic<uint64_t> totalBytesReceived;
    std::atomic<uint64_t> totalBytesSent;
    std::atomic<uint64_t> filesUploaded;
    std::atomic<uint64_t> filesDownloaded;
    double averageThroughput_kbps;  // mutex protected
    double peakThroughput_kbps;     // mutex protected
    double averageLatency_ms;       // mutex protected
    std::mutex mutex_;
public:
    void updateThroughput(uint64_t bytes, double duration_ms);
    void updateLatency(double latency_ms);
};
```

**Thread Safety**:
- Atomic variables for counters (no lock needed)
- Mutex for floating-point calculations (EMA)

## 🔄 Data Flow

### Client Upload Operation (PUT)
```
User → Client::putFile()
  ↓
Check connection & file existence
  ↓
metrics_.total_requests++
  ↓
Start timing (high_resolution_clock)
  ↓
ClientProtocol::request_put(filepath)
  ├─> Send CMD_PUT (0x03)
  ├─> Send filename[256]
  ├─> Send fileSize (uint64_t)
  └─> Loop: Read 8KB chunk → sendData() → track bytes
  ↓
End timing, calculate duration_us
  ↓
Update RTT with EMA: rtt_ms = (rtt_ms × 0.7) + (duration_ms × 0.3)
  ↓
Update transfer_latency_ms = duration_ms
  ↓
Return success/failure (on failure: failed_requests++)
```

### Client Download Operation (GET)
```
User → Client::getFile()
  ↓
Check connection
  ↓
metrics_.total_requests++
  ↓
Start timing
  ↓
ClientProtocol::request_get(filename, save_dir)
  ├─> Send CMD_GET (0x02)
  ├─> Send filename[256]
  ├─> Receive fileSize (uint64_t)
  └─> Loop: receiveData() → Write 8KB chunk → track bytes
  ↓
End timing, update RTT with EMA
  ↓
Calculate throughput_kbps = (bytes × 8) / (duration_ms / 1000) / 1024
  ↓
Return success/failure
```

### Server Request Processing
```
ClientSession::handleSession() - Worker Thread
  ↓
ServerProtocol::processRequest(clientFd)
  ↓
Start latency timing
  ↓
Read command byte (LIST/GET/PUT)
  ↓
  ├─> CMD_LIST: handleListCommand()
  │   ├─ Read shared directory via *sharedDirectory_
  │   ├─ Count files
  │   ├─ Send fileCount (uint32_t)
  │   └─ Send each filename[256]
  │
  ├─> CMD_GET: handleGetCommand()
  │   ├─ Receive filename[256]
  │   ├─ Open file from *sharedDirectory_
  │   ├─ Send fileSize (uint64_t)
  │   ├─ sendFile() - loop 8KB chunks
  │   ├─ metrics_->addBytesSent(totalSent)
  │   └─ metrics_->filesDownloaded++
  │
  └─> CMD_PUT: handlePutCommand()
      ├─ Receive filename[256] and fileSize
      ├─ Create file in *sharedDirectory_
      ├─ receiveFile() - loop 8KB chunks
      ├─ metrics_->addBytesReceived(totalReceived)
      └─ metrics_->filesUploaded++
  ↓
End latency timing
  ↓
metrics_->updateLatency(duration_ms)
```

### Dynamic Directory Change
```
Server Main Thread:
  User → Server::setSharedDirectory(newPath)
    ↓
  *sharedDirectory_ = newPath  // Atomic string update
    ↓
  All ClientSession threads automatically see new directory
    ↓
  Next file operation uses updated path
```

## 📊 Metrics System

### Client Metrics
Tracked in real-time for each operation:

**RTT (Round-Trip Time)**:
- Measured with microsecond precision using `std::chrono::high_resolution_clock`
- Smoothed using Exponential Moving Average (EMA): `rtt_ms = (rtt_ms × 0.7) + (new_value × 0.3)`
- Formula: α = 0.3 (30% weight to new value)

**Throughput**:
- Formula: `throughput_kbps = (bytes × 8) / (duration_ms / 1000) / 1024`
- Calculated per transfer operation
- Example: 2MB in 1000ms = 16,384 kbps

**Packet Loss Rate**:
- Formula: `packet_loss = (failed_requests / total_requests) × 100%`
- Uses atomic counters: `std::atomic<uint64_t> total_requests, failed_requests`
- Thread-safe increments

**Transfer Latency**:
- Time from start to completion of each operation
- Stored in `transfer_latency_ms`

### Server Metrics
Thread-safe tracking across all client sessions:

**Connection Metrics** (atomic):
- `totalConnections`: Cumulative connections since start
- `activeConnections`: Current active clients
- Updated atomically on session start/end

**Byte Counters** (atomic):
- `totalBytesReceived`: All bytes uploaded by clients
- `totalBytesSent`: All bytes downloaded by clients
- Atomic addition in sendFile/receiveFile

**File Counters** (atomic):
- `filesUploaded`: Total files received from clients
- `filesDownloaded`: Total files sent to clients
- Incremented after successful transfer

**Throughput Tracking** (mutex-protected):
- `averageThroughput_kbps`: Smoothed with EMA (α=0.1)
- `peakThroughput_kbps`: Maximum observed throughput
- Formula: `throughput = (bytes × 8) / (duration_ms / 1000) / 1024`
- EMA: `avg = (avg × 0.9) + (new × 0.1)`

**Latency Tracking** (mutex-protected):
- `averageLatency_ms`: Request processing time with EMA (α=0.1)
- Measured from processRequest() start to end
- EMA: `avg = (avg × 0.9) + (latency × 0.1)`

### Metrics Export
Both client and server support CSV export:
```
Timestamp,Metric,Value,Unit
2024-12-29 10:30:15,RTT,45.23,ms
2024-12-29 10:30:15,Throughput,8192.5,kbps
...
```

## 🎯 Key Design Decisions

### 1. Multi-threaded Server Architecture
**Rationale**: 
- One thread per client enables concurrent file transfers
- Isolated sessions prevent one client from blocking others
- Clean resource management with RAII and unique_ptr

### 2. Dynamic Shared Directory with shared_ptr
**Rationale**:
- All sessions automatically see directory changes
- Thread-safe without explicit locking (atomic ref counting)
- Efficient memory sharing across threads
- Example: Server changes directory from `./shared` to `./new_shared`, all active sessions immediately use new path

### 3. Thread-Safe Metrics
**Rationale**:
- Atomic variables for simple counters (no lock overhead)
- Mutex only for floating-point EMA calculations
- Lock-free reads for atomic counters
- Accurate concurrent tracking from multiple sessions

### 4. Exponential Moving Average (EMA)
**Rationale**:
- Smooth out spikes in metrics
- Recent values weighted more heavily
- Server: α=0.1 (slow adaptation for stability)
- Client: α=0.3 (faster adaptation for responsiveness)

### 5. Microsecond-Precision Timing
**Rationale**:
- Milliseconds too coarse for fast operations
- `std::chrono::high_resolution_clock` with `microseconds`
- Accurate RTT and latency measurements
- Example: 500μs operation vs 0ms with millisecond precision

### 6. Binary Protocol Design
**Rationale**:
- Compact representation (1 byte for command vs text)
- Fixed-size fields for predictable parsing
- Network byte order for portability
- Simple state machine for processing

### 7. Separate Core Components
**Rationale**:
- Single Responsibility Principle
- Socket layer isolated from protocol logic
- Protocol isolated from business logic
- Easier testing and maintenance

## 🔧 Configuration

### Client Configuration
```cpp
Client client;

// Connect to server
client.connect("127.0.0.1", 8080);

// Enable verbose output
client.setVerbose(true);

// Set connection timeout
client.setTimeout(5000);  // 5 seconds

// View metrics
client.displayMetrics();

// Export to CSV
client.exportMetrics("client_metrics.csv");
```

### Server Configuration
```cpp
Server server;

// Set port
server.start(8080);

// Set shared directory (dynamically!)
server.setSharedDirectory("./shared");  // All sessions update automatically

// Set max concurrent connections
server.setMaxConnections(10);

// Enable verbose output
server.setVerbose(true);

// Monitor metrics
server.displayMetrics();

// Export to CSV
server.exportMetrics("server_metrics.csv");

// Change directory at runtime
server.setSharedDirectory("./new_folder");  // Immediate effect!
```

## 🧪 Testing Strategy

### Unit Tests
- Test each component in isolation
- Mock socket I/O for protocol testing
- Test atomic operations for metrics
- Test EMA calculations

### Integration Tests
- Test client-server communication
- Test multi-threaded scenarios
- Test dynamic directory changes
- Test metrics accuracy under load

### Example Test Cases:
```cpp
// Test dynamic directory
Server server;
ClientSession session1, session2;
server.setSharedDirectory("./shared");
// Verify both sessions see ./shared
server.setSharedDirectory("./new");
// Verify both sessions immediately see ./new

// Test thread-safe metrics
ServerMetrics metrics;
std::thread t1([&]() { metrics.filesUploaded++; });
std::thread t2([&]() { metrics.filesUploaded++; });
// Verify final count is exactly 2

// Test EMA calculation
ClientMetrics metrics;
// First value: RTT should equal input
metrics.updateRTT(100.0);
assert(metrics.rtt_ms == 100.0);
// Second value: EMA should apply
metrics.updateRTT(200.0);
assert(metrics.rtt_ms == 130.0);  // (100 * 0.7) + (200 * 0.3)
```

## 📈 Performance Considerations

### Transfer Chunk Size
Current: **8KB (8192 bytes)** - balanced for most scenarios
- Small files (<1MB): Fast, low latency
- Medium files (1-100MB): Good throughput
- Large files (>100MB): Efficient, predictable memory usage

### Memory Usage
- **Client**: ~16KB per connection (socket buffers + metrics)
- **Server per session**: ~24KB (socket buffers + metrics + thread stack)
- **Total server**: Base + (24KB × active connections)

### Concurrency
- **Server threads**: One per client + main accept thread
- **Max connections**: Configurable (default: unlimited, recommended: 10-100)
- **Thread safety**: All shared state properly synchronized

### Metrics Overhead
- **Atomic operations**: ~5-10 CPU cycles (negligible)
- **EMA calculations**: Mutex-protected, ~100ns per update
- **Total overhead**: <0.1% of transfer time

### Network Optimization
- **TCP_NODELAY**: Not set (Nagle's algorithm enabled for efficiency)
- **Socket buffer**: OS default (typically 128KB-256KB)
- **Blocking I/O**: Simple, reliable for file transfer use case
- **No additional buffering**: Direct file → socket → file

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

### Basic Client Usage
```cpp
#include "client.h"

Client client;

// Connect
if (!client.connect("127.0.0.1", 8080)) {
    std::cerr << "Connection failed" << std::endl;
    return 1;
}

// List files
std::vector<std::string> files;
if (client.listFiles(files)) {
    for (const auto& file : files) {
        std::cout << file << std::endl;
    }
}

// Upload file
if (!client.putFile("local_file.txt")) {
    std::cerr << "Upload failed" << std::endl;
}

// Download file
if (!client.getFile("remote_file.txt", "./downloads")) {
    std::cerr << "Download failed" << std::endl;
}

// View metrics
client.displayMetrics();

// Disconnect
client.disconnect();
```

### Basic Server Usage
```cpp
#include "server.h"

Server server;

// Set shared directory
server.setSharedDirectory("./shared");

// Set max connections
server.setMaxConnections(10);

// Enable verbose mode
server.setVerbose(true);

// Start server (blocks until stop)
server.start(8080);

// In another thread or signal handler:
// server.stop();
```

### Interactive Server with Runtime Control
```cpp
Server server;
server.setSharedDirectory("./shared");

std::thread serverThread([&]() {
    server.start(8080);
});

// Command loop
while (true) {
    std::string cmd;
    std::cin >> cmd;
    
    if (cmd == "stop") {
        server.stop();
        break;
    } else if (cmd == "metrics") {
        server.displayMetrics();
    } else if (cmd == "setdir") {
        std::string newDir;
        std::cin >> newDir;
        server.setSharedDirectory(newDir);
        std::cout << "Directory changed! All sessions updated." << std::endl;
    }
}

serverThread.join();
```

## 📚 Dependencies

### Required
- **C++11 or higher** (C++14 recommended for full features)
- **POSIX socket API** (Linux, macOS, Unix-like systems)
- **pthread library** (multi-threading support)
- **Standard C++ libraries**: `<atomic>`, `<thread>`, `<mutex>`, `<chrono>`, `<memory>`

### Build System
- **CMake 3.10+** (recommended)
- **GNU Make** (alternative)
- **g++ 7+** or **clang++ 5+**

### Optional
- **gtest/gmock** (for unit testing)
- **Valgrind** (for memory leak detection)

## 🔮 Future Enhancements

### High Priority
1. **Resume Support**: Continue interrupted transfers using byte ranges
2. **Compression**: gzip/zlib compression for text files
3. **Checksums**: MD5/SHA256 verification after transfer
4. **Authentication**: Username/password or token-based auth

### Medium Priority
5. **TLS/SSL Encryption**: Secure data transmission
6. **Batch Operations**: Transfer multiple files in one command
7. **Progress Callbacks**: Real-time progress updates for UI
8. **Bandwidth Throttling**: Configurable rate limiting

### Low Priority
9. **HTTP REST API**: Alternative to binary protocol
10. **File Synchronization**: Bidirectional sync like rsync
11. **Compression Negotiation**: Client/server agree on compression
12. **Concurrent Transfers**: Multiple files in parallel per client

## 📊 Thread Safety Summary

| Component | Thread Safety Mechanism | Access Pattern |
|-----------|------------------------|----------------|
| ServerMetrics counters | `std::atomic<uint64_t>` | Lock-free concurrent writes |
| ServerMetrics EMA | `std::mutex` | Rare, short critical sections |
| sharedDirectory_ | `std::shared_ptr<std::string>` | Atomic refcount, immutable reads |
| ClientSession | Thread-per-session | Isolated, no sharing |
| Client socket_ | Not thread-safe | Single-threaded client use |

## 📝 API Reference

### Client API
```cpp
class Client {
public:
    bool connect(const std::string& ip, uint16_t port);
    void disconnect();
    bool isConnected() const;
    
    bool listFiles(std::vector<std::string>& files);
    bool getFile(const std::string& filename, const std::string& save_dir);
    bool putFile(const std::string& filepath);
    
    void displayMetrics() const;
    void exportMetrics(const std::string& filename) const;
    ClientMetrics getMetrics() const;
    
    void setTimeout(int timeout_ms);
    void setVerbose(bool verbose);
};
```

### Server API
```cpp
class Server {
public:
    void start(uint16_t port);
    void stop();
    bool isRunning() const;
    
    void setSharedDirectory(const std::string& directory);
    std::string getSharedDirectory() const;
    void setMaxConnections(size_t max);
    
    void displayMetrics() const;
    void exportMetrics(const std::string& filename) const;
    ServerMetrics getMetrics() const;
    
    void setVerbose(bool verbose);
};
```

See [METRICS_DOCUMENTATION.md](METRICS_DOCUMENTATION.md) for detailed metrics documentation.
