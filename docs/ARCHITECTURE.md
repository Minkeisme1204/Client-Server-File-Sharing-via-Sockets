# 🏗️ System Architecture Documentation

## Overview

The Client-Server File Transfer System is a production-ready application built with modern C++ featuring multi-threaded server architecture, thread-safe metrics tracking, dynamic configuration, and comprehensive performance monitoring. The system uses a binary protocol over TCP sockets with support for concurrent client connections.

## Architecture Principles

1. **Multi-threaded Server**: One thread per client session for concurrent file transfers
2. **Thread-Safe Design**: Atomic operations and mutex protection for shared state
3. **Dynamic Configuration**: Runtime directory changes via shared_ptr
4. **Static Library Build**: Core functionality in `libfiletransfer.a`
5. **Comprehensive Metrics**: Real-time tracking with Exponential Moving Average (EMA)
6. **Binary Protocol**: Efficient file transfer with integrity preservation
7. **Modern C++11/14**: Smart pointers, atomic operations, threading

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                      Application Layer                              │
├──────────────────────┬──────────────────────────────────────────────┤
│   server_test.cpp    │         client_test.cpp                      │
│  (Interactive CLI)   │       (Interactive CLI)                      │
│  - Runtime commands  │       - File operations                      │
│  - Metrics display   │       - Metrics tracking                     │
│  - Dynamic config    │       - Connection management                │
└──────────────────────┴──────────────────────────────────────────────┘
           ↓                              ↓
┌─────────────────────────────────────────────────────────────────────┐
│                   Core Library Layer                                │
│              libfiletransfer.a (Static Library)                     │
├──────────────────────┬──────────────────────────────────────────────┤
│   Server             │         Client                               │
│   - Multi-threading  │       - Single connection                    │
│   - Session mgmt     │       - Protocol client                      │
│   - Shared directory │       - Metrics (atomic)                     │
│   - Metrics (atomic) │       - Timeout handling                     │
└──────────────────────┴──────────────────────────────────────────────┘
           ↓                              ↓
┌─────────────────────────────────────────────────────────────────────┐
│              Protocol & Session Layer                               │
├──────────────────────┬──────────────────────────────────────────────┤
│  ServerProtocol      │      ClientProtocol                          │
│  - LIST/GET/PUT      │      - LIST/GET/PUT                          │
│  - shared_ptr<dir>   │      - Metrics updates                       │
│  - Metrics updates   │      - Binary protocol                       │
│                      │                                              │
│  ClientSession       │      ClientMetrics                           │
│  - Per-client thread │      - RTT (EMA α=0.3)                       │
│  - Isolated protocol │      - Throughput                            │
│  - Auto cleanup      │      - Packet loss                           │
│                      │      - Atomic counters                       │
│  ServerMetrics       │                                              │
│  - Atomic counters   │                                              │
│  - EMA (α=0.1)       │                                              │
│  - Mutex for floats  │                                              │
└──────────────────────┴──────────────────────────────────────────────┘
           ↓                              ↓
┌─────────────────────────────────────────────────────────────────────┐
│                   Socket Layer                                      │
├──────────────────────┬──────────────────────────────────────────────┤
│  ServerSocket        │      ClientSocket                            │
│  - bind & listen     │      - connect                               │
│  - accept clients    │      - send/receive                          │
│  - static I/O funcs  │      - connection state                      │
└──────────────────────┴──────────────────────────────────────────────┘
           ↓                              ↓
┌─────────────────────────────────────────────────────────────────────┐
│                   System Layer                                      │
│      POSIX Sockets, pthread, std::atomic, std::chrono               │
└─────────────────────────────────────────────────────────────────────┘
```

## Component Details

### 1. Application Layer

#### server_test.cpp
**Purpose**: Production-ready multi-threaded server with interactive CLI

**Features**:
- Interactive command shell with real-time control
- Signal handling (SIGINT/SIGTERM) for graceful shutdown
- Dedicated accept thread + per-client session threads
- Dynamic shared directory changes (affects all active sessions)
- Real-time metrics display with CSV export
- Thread-safe configuration management

**Commands**:
- `metrics` - Display comprehensive server statistics
- `export <filename>` - Export metrics to CSV file
- `setdir <path>` - Change shared directory (dynamic, affects all sessions!)
- `verbose on|off` - Toggle verbose logging
- `help` - Show all available commands
- `quit` / `Ctrl+C` - Graceful shutdown with session cleanup

**Architecture**:
```cpp
Main Thread
  ├─→ Server::start(port) spawns accept thread
  └─→ Command loop processes user input
      
Accept Thread
  ├─→ Waits for connections
  └─→ Creates ClientSession per client (spawns worker thread)
      
ClientSession Threads (one per client)
  └─→ Handles requests until disconnect
```

#### client_test.cpp
**Purpose**: Production-ready client with comprehensive metrics tracking

**Features**:
- Microsecond-precision timing for accurate RTT
- Exponential Moving Average (α=0.3) for smooth metrics
- Packet loss rate calculation
- CSV export capability
- File validation before operations
- Verbose mode for debugging

**Commands**:
- `connect <ip> <port>` - Establish connection to server
- `list` - List all files on server
- `get <filename> [save_dir]` - Download file (default: current directory)
- `put <filepath>` - Upload file to server
- `metrics` - Display client metrics (RTT, throughput, packet loss)
- `export <filename>` - Export metrics to CSV
- `verbose on|off` - Toggle verbose logging
- `disconnect` - Close connection gracefully
- `help` - Show all commands
- `quit` - Exit client

**Metrics Tracked**:
- RTT (Round-Trip Time) with EMA
- Throughput (kbps)
- Packet Loss Rate (%)
- Transfer Latency (ms)

### 2. Core Library Layer

#### Server (server.h / server.cpp)
**Responsibilities**:
- Multi-threaded server lifecycle management
- Accept loop in dedicated thread
- Session management with automatic cleanup
- Dynamic shared directory (shared_ptr - all sessions auto-update)
- Thread-safe metrics aggregation

**Key Members**:
```cpp
class Server {
private:
    std::unique_ptr<ServerSocket> socket_;
    std::unique_ptr<ServerProtocol> protocol_;
    ServerMetrics metrics_;  // Thread-safe with atomic/mutex
    std::vector<std::unique_ptr<ClientSession>> sessions_;
    std::shared_ptr<std::string> sharedDirectory_;  // Dynamic!
    std::atomic<bool> running_;
    size_t maxConnections_;
    bool verbose_;
};
```

**Key Methods**:
```cpp
void start(uint16_t port);  // Start server and accept loop
void stop();                 // Graceful shutdown
void setSharedDirectory(const std::string& dir);  // Runtime change!
ServerMetrics getMetrics() const;
void displayMetrics() const;
void exportMetrics(const std::string& filename) const;
```

**Thread Safety**:
- `sharedDirectory_` is `shared_ptr<string>` - all sessions share pointer
- When changed: `*sharedDirectory_ = newPath` - all threads see update
- `metrics_` uses atomic counters + mutex for EMA calculations
- `running_` is `atomic<bool>` for clean shutdown

#### Client (client.h / client.cpp)
**Responsibilities**:
- Single-threaded client connection management
- File operation orchestration (list, get, put)
- High-precision metrics tracking (microseconds)
- Timeout handling
- Protocol interaction

**Key Members**:
```cpp
class Client {
private:
    std::unique_ptr<ClientSocket> socket_;
    std::unique_ptr<ClientProtocol> protocol_;
    ClientMetrics metrics_;  // Contains atomic counters
    int timeout_;
    bool verbose_;
};
```

**Key Methods**:
```cpp
bool connect(const std::string& ip, uint16_t port);
void disconnect();
bool listFiles(std::vector<std::string>& files);
bool getFile(const std::string& filename, const std::string& saveDir);
bool putFile(const std::string& filepath);
ClientMetrics getMetrics() const;
void displayMetrics() const;
void exportMetrics(const std::string& filename) const;
```

**Metrics Calculation**:
- RTT: `rtt_ms = (rtt_ms × 0.7) + (new_value × 0.3)` (EMA with α=0.3)
- Throughput: `(bytes × 8) / (duration_ms / 1000) / 1024` kbps
- Packet Loss: `(failed_requests / total_requests) × 100%`

### 3. Protocol & Session Layer

#### ClientSession (core/Server/client_session.h/cpp)
**Purpose**: Handle individual client connection in isolated thread

**Key Members**:
```cpp
class ClientSession {
private:
    int clientFd_;
    std::string clientAddress_;
    std::shared_ptr<std::string> sharedDirectory_;  // Points to Server's!
    ServerMetrics* metrics_;  // Shared across all sessions
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> active_;
};
```

**Lifecycle**:
```cpp
// In Server::acceptLoop()
auto session = std::make_unique<ClientSession>(
    clientFd, 
    clientAddr, 
    sharedDirectory_,  // Pass shared_ptr
    &metrics_          // Pass metrics pointer
);
session->start();  // Spawns thread with handleSession()
```

**Thread Behavior**:
- Runs in loop until client disconnects or error
- Creates own ServerProtocol instance
- Updates shared metrics atomically
- Reads directory via `*sharedDirectory_`
- Automatic cleanup on thread exit

#### ServerProtocol (core/Server/server_protocol.h/cpp)
**Purpose**: Process client requests and execute file operations

**Key Members**:
```cpp
class ServerProtocol {
private:
    std::shared_ptr<std::string> sharedDirectory_;  // Dynamic directory
    ServerMetrics* metrics_;  // For tracking
};
```

**Key Methods**:
```cpp
void processRequest(int clientFd);  // Main request handler
void setSharedDirectoryPtr(std::shared_ptr<std::string> dir);
void setMetrics(ServerMetrics* metrics);
```

**Request Processing**:
1. Read command byte (LIST=0x01, GET=0x02, PUT=0x03)
2. Start latency timer
3. Execute command:
   - `handleListCommand()` - Lists files in `*sharedDirectory_`
   - `handleGetCommand()` - Sends file, updates `metrics_->filesDownloaded++`
   - `handlePutCommand()` - Receives file, updates `metrics_->filesUploaded++`
4. End latency timer
5. `metrics_->updateLatency(duration_ms)`

#### ClientProtocol (core/Client/client_protocol.h/cpp)
**Purpose**: Implement client-side binary protocol

**Key Methods**:
```cpp
bool request_list(std::vector<std::string>& files);
bool request_get(const std::string& filename, const std::string& saveDir);
bool request_put(const std::string& filepath);
void setMetrics(ClientMetrics* metrics);
```

**Protocol Implementation**:
- Sends command byte + parameters
- Handles responses
- Updates metrics during transfer
- 8KB chunks for file transfers

#### ServerMetrics (core/Server/server_metrics.h/cpp)
**Purpose**: Thread-safe metrics aggregation across all sessions

**Key Members**:
```cpp
class ServerMetrics {
private:
    // Atomic counters (lock-free)
    std::atomic<uint64_t> totalConnections{0};
    std::atomic<uint64_t> activeConnections{0};
    std::atomic<uint64_t> totalBytesReceived{0};
    std::atomic<uint64_t> totalBytesSent{0};
    std::atomic<uint64_t> filesUploaded{0};
    std::atomic<uint64_t> filesDownloaded{0};
    
    // Mutex-protected (for floating-point EMA)
    double averageThroughput_kbps{0.0};
    double peakThroughput_kbps{0.0};
    double averageLatency_ms{0.0};
    std::mutex mutex_;
};
```

**Key Methods**:
```cpp
void incrementConnections();
void decrementActiveConnections();
void addBytesReceived(uint64_t bytes);
void addBytesSent(uint64_t bytes);
void updateThroughput(uint64_t bytes, double duration_ms);
void updateLatency(double latency_ms);
```

**Thread Safety**:
- Atomic operations for counters (no locks needed)
- Mutex only for EMA calculations
- Formula: `avg = (avg × 0.9) + (new_value × 0.1)` (α=0.1)

#### ClientMetrics (core/Client/client_metrics.h)
**Purpose**: Client-side metrics structure with atomic counters

**Structure**:
```cpp
struct ClientMetrics {
    double rtt_ms{0.0};                    // Round-Trip Time (EMA)
    double throughput_kbps{0.0};            // Transfer speed
    double packet_loss_rate{0.0};           // Failure rate
    double transfer_latency_ms{0.0};        // Last operation time
    std::atomic<uint64_t> total_requests{0};   // Total operations
    std::atomic<uint64_t> failed_requests{0};  // Failed operations
};
```

**Calculations**:
- RTT EMA: `rtt_ms = (rtt_ms × 0.7) + (new_rtt × 0.3)` (α=0.3)
- Packet Loss: `(failed_requests / total_requests) × 100%`
- Throughput: `(bytes × 8) / (duration_ms / 1000) / 1024` kbps

### 4. Socket Layer

#### ServerSocket (core/Server/server_socket.h/cpp)
**Purpose**: Server socket management

**Features**:
- Socket creation with SO_REUSEADDR
- Bind to port
- Listen for connections
- Accept with client address extraction
- Static I/O methods for sessions

**Key Methods**:
```cpp
bool bind(uint16_t port);
bool listen(int backlog = 10);
int acceptConnection(std::string& clientAddr);
static ssize_t sendData(int sockfd, const uint8_t* data, size_t size);
static ssize_t receiveData(int sockfd, uint8_t* buffer, size_t size);
void close();
```

#### ClientSocket (core/Client/client_socket.h/cpp)
**Purpose**: Client socket management

**Features**:
- Connect to server
- Send/receive with full blocking I/O
- Connection state tracking
- Automatic cleanup

**Key Methods**:
```cpp
bool connectToServer(const std::string& ip, uint16_t port);
void disconnect();
ssize_t sendData(const uint8_t* data, size_t size);
ssize_t receiveData(uint8_t* buffer, size_t size);
bool isConnected() const;
```

## Protocol Design

### Command Codes
```cpp
#define CMD_LIST 0x01  // List files in shared directory
#define CMD_GET  0x02  // Download file from server
#define CMD_PUT  0x03  // Upload file to server
```

### Message Formats

#### LIST Command
```
Client → Server: 
  [CMD_LIST (1 byte)]

Server → Client: 
  [File Count (uint32_t, 4 bytes)]
  [Filename 1 (char[256])]
  [Filename 2 (char[256])]
  ...
  [Filename N (char[256])]
```

#### GET Command
```
Client → Server:
  [CMD_GET (1 byte)]
  [Filename (char[256])]

Server → Client:
  [File Size (uint64_t, 8 bytes)]
  [File Data (chunks of 8192 bytes)]
```

**Server Behavior**:
1. Read filename
2. Construct path: `*sharedDirectory_ + "/" + filename`
3. Open file, get size via `stat()`
4. Send file size
5. Loop: read 8KB → send → `addBytesSent()`
6. `filesDownloaded++`

#### PUT Command
```
Client → Server:
  [CMD_PUT (1 byte)]
  [Filename (char[256])]
  [File Size (uint64_t, 8 bytes)]
  [File Data (chunks of 8192 bytes)]

Server: (No response, success assumed if no disconnect)
```

**Server Behavior**:
1. Read filename and file size
2. Construct path: `*sharedDirectory_ + "/" + filename`
3. Create/open file for writing
4. Loop: receive 8KB → write → `addBytesReceived()`
5. `filesUploaded++`

### Binary Protocol Details
- **Endianness**: Host byte order (assumes same architecture)
- **Chunk Size**: 8192 bytes (8KB) for all transfers
- **Filename Length**: Fixed 256 bytes (null-terminated C string)
- **File Size**: uint64_t (supports files up to ~18 EB)

## Threading Model

### Server Threading Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Main Thread                            │
│  - Parse command-line arguments                             │
│  - Create Server instance                                   │
│  - Call server.start(port) → spawns Accept Thread          │
│  - Enter command loop (metrics, setdir, quit, etc.)        │
│  - Handle Ctrl+C signal → server.stop()                    │
└─────────────────────────────────────────────────────────────┘
                          │
                          │ spawns
                          ▼
┌─────────────────────────────────────────────────────────────┐
│                    Accept Thread                            │
│  while (running_) {                                         │
│    - Call socket_->acceptConnection()                       │
│    - If success:                                            │
│      • metrics_.incrementConnections()                      │
│      • Create ClientSession(fd, addr, sharedDirectory_,     │
│                              &metrics_)                     │
│      • session->start() → spawns Worker Thread              │
│      • sessions_.push_back(session)                         │
│    - cleanupFinishedSessions()                              │
│  }                                                           │
└─────────────────────────────────────────────────────────────┘
                          │
                          │ spawns (one per client)
                          ▼
┌─────────────────────────────────────────────────────────────┐
│              ClientSession Worker Threads                   │
│  (One thread per connected client)                          │
│                                                              │
│  void handleSession() {                                     │
│    - Create ServerProtocol instance                         │
│    - protocol.setSharedDirectoryPtr(sharedDirectory_)       │
│    - protocol.setMetrics(metrics_)                          │
│    - while (active_) {                                      │
│        protocol.processRequest(clientFd_)                   │
│      }                                                       │
│    - cleanup()                                              │
│  }                                                           │
└─────────────────────────────────────────────────────────────┘
```

### Thread Synchronization

**Shared Resources**:
1. **sharedDirectory_** (`std::shared_ptr<std::string>`)
   - Shared across all ClientSession threads
   - Read via `*sharedDirectory_`
   - Modified only by main thread: `*sharedDirectory_ = newPath`
   - Thread-safe: atomic reference counting + string immutability per read

2. **metrics_** (`ServerMetrics`)
   - Shared pointer across all sessions
   - Atomic operations for counters (no locks)
   - Mutex-protected for EMA calculations

3. **running_** (`std::atomic<bool>`)
   - Atomic flag for clean shutdown coordination
   - Set by main thread, read by accept thread

4. **sessions_** (`std::vector<std::unique_ptr<ClientSession>>`)
   - Managed by accept thread
   - No concurrent access (only accept thread modifies)

### Client Threading

Client is **single-threaded**:
- One connection per client instance
- Blocking I/O operations
- Sequential request-response model
- Timing done in main thread with `std::chrono::high_resolution_clock`

## Data Flow

### File Upload (PUT Command)

```
Client                                    Server (ClientSession Thread)
  │                                          │
  ├─ metrics_.total_requests++              │
  ├─ Start timing (microseconds)            │
  │                                          │
  ├─ Send CMD_PUT (0x03) ──────────────────►│
  ├─ Send filename[256] ────────────────────►│
  ├─ Send fileSize (uint64_t) ──────────────►│
  │                                          ├─ Read *sharedDirectory_
  │                                          ├─ Create file: sharedDir/filename
  │                                          ├─ Open for writing
  │                                          │
  ├─ Loop (while bytes remain):             │
  │   ├─ Read 8KB from file                 │
  │   ├─ Send chunk ──────────────────────► │ ├─ Receive chunk
  │   └─ Repeat                              │ ├─ Write to disk
  │                                          │ ├─ totalReceived += bytes
  │                                          │ └─ Repeat
  │                                          │
  │◄─────────────────────────────────────────┤ Close file
  │                                          ├─ metrics_->addBytesReceived(total)
  │                                          ├─ metrics_->filesUploaded++
  │                                          │
  ├─ End timing                              │
  ├─ Calculate duration_us                  │
  ├─ Update RTT with EMA (α=0.3)            │
  ├─ transfer_latency_ms = duration         │
  └─ Return success                          │
  
  On failure:
  ├─ metrics_.failed_requests++             │
  └─ Return false                            │
```

### File Download (GET Command)

```
Client                                    Server (ClientSession Thread)
  │                                          │
  ├─ metrics_.total_requests++              │
  ├─ Start timing (microseconds)            │
  │                                          │
  ├─ Send CMD_GET (0x02) ───────────────────►│
  ├─ Send filename[256] ─────────────────────►│
  │                                          ├─ Read *sharedDirectory_
  │                                          ├─ Open file: sharedDir/filename
  │                                          ├─ stat() to get file size
  │                                          │
  │◄─ Receive fileSize (uint64_t) ───────────┤ Send file size
  │                                          │
  ├─ Create local file                      │
  ├─ Open for writing                       │
  │                                          │
  ├─ Loop (while bytes remain):             │
  │◄──── Receive chunk ──────────────────────┤ ├─ Read 8KB from file
  │   ├─ Write to disk                      │ ├─ Send chunk
  │   ├─ totalReceived += bytes             │ ├─ totalSent += bytes
  │   └─ Repeat                              │ └─ Repeat
  │                                          │
  │                                          ├─ Close file
  │                                          ├─ metrics_->addBytesSent(total)
  │                                          ├─ metrics_->filesDownloaded++
  │                                          │
  ├─ Close file                              │
  ├─ End timing                              │
  ├─ Calculate duration_us                  │
  ├─ Update RTT with EMA (α=0.3)            │
  ├─ Calculate throughput_kbps              │
  ├─ transfer_latency_ms = duration         │
  └─ Return success                          │
```

### File Listing (LIST Command)

```
Client                                    Server (ClientSession Thread)
  │                                          │
  ├─ metrics_.total_requests++              │
  ├─ Start timing                            │
  │                                          │
  ├─ Send CMD_LIST (0x01) ──────────────────►│
  │                                          ├─ Read *sharedDirectory_
  │                                          ├─ opendir(sharedDir)
  │                                          ├─ Count files
  │                                          │
  │◄─ Receive fileCount (uint32_t) ──────────┤ Send count
  │                                          │
  ├─ Loop (for each file):                  │
  │◄──── Receive filename[256] ───────────────┤ ├─ Send filename
  │   ├─ Store in vector                    │ └─ Repeat
  │   └─ Repeat                              │
  │                                          ├─ closedir()
  │                                          │
  ├─ End timing                              │
  ├─ Update RTT with EMA                    │
  └─ Return success with file list          │
```

### Dynamic Directory Change Flow

```
Main Thread (server_test.cpp)                All ClientSession Threads
  │                                               │
  User types: setdir /new/path                    │
  │                                               │
  ├─ server.setSharedDirectory("/new/path")       │
  │     │                                         │
  │     ├─ *sharedDirectory_ = "/new/path"        │
  │     └─ String update is atomic                │
  │                                               │
  │                                               ├─ Next request arrives
  │                                               ├─ Read *sharedDirectory_
  │                                               ├─ Now sees "/new/path"!
  │                                               └─ Uses new directory
  │                                               
  All sessions instantly use new directory without restart or reconnection!
```

## Error Handling & Shutdown

### Graceful Shutdown Sequence

#### Server Shutdown
```
1. User presses Ctrl+C or types 'quit'
   ↓
2. Signal handler (SIGINT/SIGTERM) or command sets shutdown flag
   ↓
3. server.stop() is called
   ↓
4. Set running_ = false (atomic)
   ↓
5. Close server socket (unblocks accept thread)
   ↓
6. Accept thread exits accept loop
   ↓
7. For each ClientSession:
   - Call session->stop()
   - Set active_ = false
   - Join or detach thread
   ↓
8. Clean up sessions vector
   ↓
9. Display final metrics
   ↓
10. Main thread exits
```

#### Client Disconnect
```
1. User types 'disconnect' or 'quit'
   ↓
2. client.disconnect() is called
   ↓
3. Close socket
   ↓
4. Update connection state
   ↓
5. Server detects closed connection
   ↓
6. ClientSession thread exits loop
   ↓
7. metrics_->decrementActiveConnections()
   ↓
8. Thread cleanup
```

### Exception Safety

**Protocol Level**:
```cpp
try {
    protocol.processRequest(clientFd);
} catch (const std::exception& e) {
    if (verbose_) {
        std::cerr << "Error in session: " << e.what() << std::endl;
    }
    // Thread will exit, RAII cleans up
}
```

**Socket Level**:
- RAII for socket file descriptors
- Automatic close() in destructors
- No manual resource management

**Thread Level**:
- unique_ptr for thread management
- Automatic join/detach on destruction
- No memory leaks on exceptions

### Error Propagation

**Return Values**:
- `bool` for simple success/failure
- `false` indicates error, check verbose output

**Metrics Updates**:
- `failed_requests++` on client errors
- No exception throwing across thread boundaries

## Build System Architecture

### CMake Structure

```
CMakeLists.txt (root)
  ├─ Project: Client-Server-File-Sharing
  ├─ C++11 required (with C++14 features)
  ├─ Find pthread library
  │
  ├─ Build Static Library: libfiletransfer.a
  │   ├─ Sources: src/**/*.cpp
  │   └─ Headers: include/**/*.h
  │
  ├─ Build Executable: server_test
  │   ├─ Source: tests/server_test.cpp
  │   └─ Link: libfiletransfer.a + pthread
  │
  ├─ Build Executable: client_test
  │   ├─ Source: tests/client_test.cpp
  │   └─ Link: libfiletransfer.a + pthread
  │
  ├─ Build Executable: simp_server
  │   ├─ Source: tests/simple_server.cpp
  │   └─ Link: libfiletransfer.a + pthread
  │
  └─ Build Executable: simp_client
      ├─ Source: tests/simple_client.cpp
      └─ Link: libfiletransfer.a + pthread
```

### Source Organization

```
src/
├── client.cpp                   # Client high-level API
├── server.cpp                   # Server high-level API
└── core/
    ├── Client/
    │   ├── client_socket.cpp    # Socket management
    │   ├── client_protocol.cpp  # Protocol implementation
    │   └── client_metrics.cpp   # Metrics calculations
    └── Server/
        ├── server_socket.cpp    # Server socket
        ├── server_protocol.cpp  # Request processing
        ├── server_metrics.cpp   # Metrics aggregation
        └── client_session.cpp   # Per-client thread handler

include/
├── client.h                     # Client public API
├── server.h                     # Server public API
└── core/
    ├── Client/
    │   ├── client_socket.h
    │   ├── client_protocol.h
    │   └── client_metrics.h
    └── Server/
        ├── server_socket.h
        ├── server_protocol.h
        ├── server_metrics.h
        └── client_session.h
```

### Dependencies

**Required**:
- **C++11 compiler** (g++ 7+, clang++ 5+)
- **pthread library** (POSIX threads)
- **POSIX sockets** (Linux, macOS, Unix-like systems)

**Standard Library Features Used**:
- `<atomic>` - Thread-safe counters
- `<thread>` - Multi-threading
- `<mutex>` - Synchronization
- `<chrono>` - High-resolution timing
- `<memory>` - Smart pointers (unique_ptr, shared_ptr)
- `<vector>`, `<string>`, `<fstream>` - Basic containers

**Build Output**:
```
build/
├── libfiletransfer.a      # Static library
├── server_test            # Interactive server
├── client_test            # Interactive client
├── simp_server            # Simple server example
└── simp_client            # Simple client example
```

## Performance Characteristics

### Transfer Performance

**Chunk Size**: 8192 bytes (8KB)
- Balanced for throughput and memory usage
- Single chunk fits in typical L1 cache
- Minimizes system call overhead

**Throughput Calculation**:
```
throughput_kbps = (bytes × 8) / (duration_ms / 1000) / 1024

Example: 10MB file in 5 seconds
= (10,485,760 × 8) / 5 / 1024
= 16,384 kbps (16 Mbps)
```

**Timing Precision**:
- Client: `std::chrono::high_resolution_clock` with microseconds
- Typical precision: 1μs on modern systems
- Accurate for operations as fast as 100μs

### Concurrency & Scalability

**Server Thread Model**:
- Main thread: Command processing
- Accept thread: Connection handling
- Worker threads: One per client (N clients = N threads)

**Scalability Limits**:
```
Theoretical maximum clients = System thread limit
Linux default: ~1000-2000 threads per process
Practical limit: ~100 clients for smooth operation
```

**Memory Usage**:
- Server base: ~2MB (process overhead)
- Per client session: ~24KB
  - Thread stack: ~8KB default
  - Socket buffers: ~16KB (8KB send + 8KB receive)
  - Protocol overhead: <1KB
- 100 clients: ~2MB + (100 × 24KB) = ~4.4MB total

**CPU Usage**:
- Idle: <1% (just accept thread polling)
- Active transfer: ~5-10% per active client
- 10 concurrent transfers: ~50-100% CPU usage

### Metrics Overhead

**Atomic Operations**:
- Increment: ~5-10 CPU cycles
- Compare-and-swap: ~20-30 cycles
- Negligible compared to I/O time (microseconds vs milliseconds)

**EMA Calculations**:
- Mutex lock: ~100ns
- Float multiplication: ~10ns
- Total per update: ~200ns
- Percentage of 1ms operation: 0.02%

**CSV Export**:
- Blocking operation (mutex held)
- Duration: ~1ms for typical metrics
- Recommended: Export only when requested, not during transfers

### Network Optimization

**TCP Settings**:
- `SO_REUSEADDR`: Quick server restart
- `TCP_NODELAY`: Not set (Nagle's algorithm enabled)
- Send/Receive buffers: OS default (typically 64-256KB)

**Blocking I/O**:
- Simple, reliable for file transfer use case
- No need for select/poll/epoll complexity
- Each client has dedicated thread

**No Additional Buffering**:
- Direct file → socket or socket → file
- Operating system handles buffering efficiently
- Minimal memory copies

## Security Considerations

### Current Implementation

**Authentication**: None
- No user verification
- Any client can connect

**Encryption**: None
- Plain TCP transmission
- All data sent in clear text
- File contents exposed on network

**Access Control**: Directory-based only
- Server limits access to shared directory
- No per-user or per-file permissions
- All connected clients see same files

**Input Validation**: Minimal
- Filename length limited to 256 bytes
- Path traversal not explicitly prevented (relies on directory jail)
- No file type restrictions

### Recommended for Production Use

1. **Authentication**:
   - Username/password verification
   - Token-based authentication
   - Public key authentication

2. **Encryption**:
   - TLS/SSL for all communication
   - File encryption at rest
   - Certificate verification

3. **Access Control**:
   - Per-user directory permissions
   - File-level ACLs
   - Audit logging

4. **Input Validation**:
   - Strict path traversal prevention
   - Filename sanitization
   - File size limits
   - Rate limiting

5. **Monitoring**:
   - Connection logging
   - Failed authentication tracking
   - Anomaly detection

**Current Use Case**: Educational, internal networks, trusted environments

## Design Patterns & Best Practices

### 1. RAII (Resource Acquisition Is Initialization)
**Usage**: Socket management, thread management
```cpp
// Socket closed automatically in destructor
ClientSocket socket;
socket.connectToServer("127.0.0.1", 8080);
// ... use socket ...
// Automatic cleanup on scope exit
```

### 2. Smart Pointers
**shared_ptr for Dynamic Configuration**:
```cpp
std::shared_ptr<std::string> sharedDirectory_;
// Multiple threads share pointer
// All see updates: *sharedDirectory_ = newPath
```

**unique_ptr for Ownership**:
```cpp
std::unique_ptr<ServerSocket> socket_;
std::unique_ptr<ClientSession> session;
// Clear ownership, automatic cleanup
```

### 3. Thread Safety Patterns

**Lock-Free Atomics**:
```cpp
std::atomic<uint64_t> totalConnections;
totalConnections++;  // Thread-safe, no mutex needed
```

**Mutex for Complex Operations**:
```cpp
{
    std::lock_guard<std::mutex> lock(mutex_);
    averageThroughput_kbps = (avg * 0.9) + (new_value * 0.1);
}
```

**Shared State via Pointer**:
```cpp
// Pass pointer, not reference, for thread sharing
ClientSession(int fd, ServerMetrics* metrics) 
    : metrics_(metrics) {}
```

### 4. Exponential Moving Average (EMA)

**Server (α=0.1, slow adaptation)**:
```cpp
if (averageLatency == 0.0) {
    averageLatency = latency;  // First value
} else {
    averageLatency = (averageLatency * 0.9) + (latency * 0.1);
}
```

**Client (α=0.3, faster adaptation)**:
```cpp
if (rtt_ms == 0.0) {
    rtt_ms = duration_ms;
} else {
    rtt_ms = (rtt_ms * 0.7) + (duration_ms * 0.3);
}
```

**Rationale**:
- Smooths out spikes and noise
- Recent values have more weight
- Server uses slower α for stability (more clients, more noise)
- Client uses faster α for responsiveness (single connection)

### 5. Thread-Per-Client vs Thread Pool

**Current: Thread-Per-Client**
- Simple implementation
- No task queue complexity
- Good for <100 clients

**Future: Thread Pool**
- Fixed number of worker threads
- Task queue for requests
- Better for 100+ clients

## Testing Strategy

### Unit Testing
**Components to Test**:
- ClientSocket: connect, send, receive
- ServerSocket: bind, listen, accept
- ClientProtocol: message formatting
- ServerProtocol: request processing
- Metrics: atomic operations, EMA calculations

**Example Test**:
```cpp
TEST(ClientMetrics, RTT_EMA) {
    ClientMetrics metrics;
    metrics.rtt_ms = 100.0;
    
    // Simulate new measurement
    double new_rtt = 200.0;
    metrics.rtt_ms = (metrics.rtt_ms * 0.7) + (new_rtt * 0.3);
    
    ASSERT_DOUBLE_EQ(metrics.rtt_ms, 130.0);  // (100*0.7) + (200*0.3)
}
```

### Integration Testing
**Scenarios**:
1. Start server, connect client, transfer file
2. Multiple clients simultaneously
3. Dynamic directory change during transfer
4. Graceful shutdown with active connections
5. Metrics accuracy verification

### Stress Testing
**Test Cases**:
- 100 simultaneous clients
- Large file transfers (>1GB)
- Rapid connect/disconnect cycles
- Directory changes under load

## Future Enhancements

### High Priority
1. **TLS/SSL Support**: Encrypted communication
2. **Authentication**: Username/password or certificates
3. **Compression**: gzip/zlib for text files
4. **Resume Support**: Partial transfer continuation

### Medium Priority
5. **Thread Pool**: Replace thread-per-client
6. **Bandwidth Limiting**: Rate control per client
7. **File Checksums**: MD5/SHA256 verification
8. **Batch Operations**: Multiple files in one command

### Low Priority
9. **HTTP REST API**: Alternative to binary protocol
10. **WebSocket Support**: For web clients
11. **File Synchronization**: Bidirectional sync
12. **Delta Transfers**: Only transfer changes

## Troubleshooting Guide

### Common Issues

**Issue**: "Address already in use"
**Solution**: Server port still bound from previous run
```bash
# Wait 60 seconds for OS to release port
# Or use different port temporarily
```

**Issue**: "Connection refused"
**Solution**: Server not running or firewall blocking
```bash
# Check if server is listening
netstat -tlnp | grep 8080
# Check firewall
sudo iptables -L
```

**Issue**: Metrics show 0 RTT
**Solution**: Operations completing too fast for millisecond timing
- Already fixed: Using microsecond precision
- Verify with verbose mode

**Issue**: High packet loss rate
**Solution**: Network issues or file not found errors
```bash
# Check verbose output for actual errors
# Verify file exists before PUT
# Check network stability with ping
```

**Issue**: Directory change not working
**Solution**: Verify path exists and has permissions
```bash
# Check directory permissions
ls -ld /path/to/shared
# Verify in verbose mode
```

## References & Further Reading

### Standards & RFCs
- [RFC 793](https://tools.ietf.org/html/rfc793) - TCP Protocol
- [RFC 768](https://tools.ietf.org/html/rfc768) - UDP Protocol (for comparison)
- [POSIX.1-2017](https://pubs.opengroup.org/onlinepubs/9699919799/) - Socket API

### Books
- "Unix Network Programming" by W. Richard Stevens
- "C++ Concurrency in Action" by Anthony Williams
- "Modern C++ Design" by Andrei Alexandrescu

### Documentation
- [CMake Documentation](https://cmake.org/documentation/)
- [C++ Reference](https://en.cppreference.com/)
- [Linux Socket Programming](https://man7.org/linux/man-pages/man7/socket.7.html)

### Project Documentation
- [METRICS_DOCUMENTATION.md](METRICS_DOCUMENTATION.md) - Detailed metrics guide
- [ARCHITECTURE_DIAGRAMS.md](ARCHITECTURE_DIAGRAMS.md) - Visual architecture
- [DESIGN.md](DESIGN.md) - Design decisions and patterns
- [README.md](README.md) - Quick start guide

---

**Document Version**: 2.0  
**Last Updated**: December 29, 2024  
**Reflects**: Current production implementation with multi-threading, dynamic configuration, and comprehensive metrics
