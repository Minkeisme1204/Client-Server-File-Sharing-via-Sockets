# 🏗️ System Architecture Documentation

## Overview

The File Transfer System is a professional client-server application built with modern C++ architecture, featuring modular design, multi-threading, and robust error handling.

## Architecture Principles

1. **Modular Design**: Separate core library and test applications
2. **Single Static Library**: Core functionality in `libfiletransfer.a`
3. **Thread-Safe**: Multi-client support with thread management
4. **Binary Protocol**: Efficient file transfer with integrity preservation
5. **Modern C++17**: Leveraging latest language features

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                         │
├──────────────────────┬──────────────────────────────────────┤
│   server_test.cpp    │         client_test.cpp              │
│  (Interactive CLI)   │       (Interactive CLI)              │
└──────────────────────┴──────────────────────────────────────┘
           ↓                              ↓
┌─────────────────────────────────────────────────────────────┐
│                   Core Library Layer                         │
│              libfiletransfer.a (Static)                      │
├──────────────────────┬──────────────────────────────────────┤
│   Server.cpp         │         Client.cpp                   │
│   (High-level API)   │       (High-level API)               │
└──────────────────────┴──────────────────────────────────────┘
           ↓                              ↓
┌─────────────────────────────────────────────────────────────┐
│              Protocol & Socket Layer                         │
├──────────────────────┬──────────────────────────────────────┤
│  ServerProtocol      │      ClientProtocol                  │
│  ServerSocket        │      ClientSocket                    │
│  ClientSession       │      ClientMetrics                   │
│  ServerMetrics       │                                      │
└──────────────────────┴──────────────────────────────────────┘
           ↓                              ↓
┌─────────────────────────────────────────────────────────────┐
│                   System Layer                               │
│         POSIX Sockets, pthread, filesystem                   │
└─────────────────────────────────────────────────────────────┘
```

## Component Details

### 1. Application Layer

#### server_test.cpp
**Purpose**: Production-ready server application with CLI

**Features**:
- Interactive command shell
- Signal handling (Ctrl+C graceful shutdown)
- Multi-threading (accept loop in separate thread)
- Real-time metrics display
- Configuration management

**Commands**:
- `metrics` - Display server statistics
- `clients` - Show active connections
- `reset` - Reset metrics
- `export` - Export to CSV
- `dir` - Change shared directory
- `verbose` - Toggle logging
- `quit` - Graceful shutdown

#### client_test.cpp
**Purpose**: Production-ready client application with CLI

**Features**:
- Auto-connect option
- File validation before upload
- Progress indicators
- Command history friendly
- Quoted string support for filenames with spaces

**Commands**:
- `connect <ip> <port>` - Connect to server
- `list` - List files
- `get <file> [dir]` - Download
- `put <file>` - Upload
- `metrics` - View stats
- `disconnect` - Close connection

### 2. Core Library Layer

#### Server (server.h/cpp)
**Responsibilities**:
- Server lifecycle management (start/stop/run)
- Accept loop in dedicated thread
- Session management and cleanup
- Configuration (port, directory, limits)

**Key Methods**:
```cpp
bool start(uint16_t port, const std::string& sharedDir);
void stop();  // Graceful shutdown
void run();   // Main accept loop
const ServerMetrics& getMetrics() const;
```

#### Client (client.h/cpp)
**Responsibilities**:
- Connection management
- File operations (list, get, put)
- Metrics tracking
- Timeout handling

**Key Methods**:
```cpp
bool connect(const std::string& ip, uint16_t port);
bool listFiles();
bool getFile(const std::string& filename, const std::string& saveDir);
bool putFile(const std::string& filepath);
const ClientMetrics& getMetrics() const;
```

### 3. Protocol & Socket Layer

#### Protocol Design

**Command Codes**:
```cpp
#define CMD_LIST 0x01  // List files
#define CMD_GET  0x02  // Download file
#define CMD_PUT  0x03  // Upload file
```

**Message Format**:

**LIST Request/Response**:
```
Client → Server: [CMD_LIST (1 byte)]
Server → Client: [File Count (4 bytes)]
                 [Filename 1 (256 bytes)]
                 [Filename 2 (256 bytes)]
                 ...
```

**GET Request/Response**:
```
Client → Server: [CMD_GET (1 byte)][Filename (256 bytes)]
Server → Client: [File Size (8 bytes)][File Data (n bytes)]
```

**PUT Request/Response**:
```
Client → Server: [CMD_PUT (1 byte)]
                 [Filename (256 bytes)]
                 [File Size (8 bytes)]
                 [File Data (n bytes)]
```

#### ClientSession (client_session.h/cpp)
**Purpose**: Handle individual client connection

**Features**:
- Dedicated thread per client
- Protocol processing loop
- Exception handling
- Automatic cleanup

**Lifecycle**:
```cpp
ClientSession session(clientFd, clientAddr, sharedDir);
session.start();  // Spawns thread
// ... connection active ...
session.stop();   // Graceful cleanup
```

### 4. Socket Layer

#### ServerSocket
**Features**:
- Socket creation and binding
- Listen for connections
- Accept with non-blocking option
- SO_REUSEADDR for quick restart

#### ClientSocket
**Features**:
- Connect to server
- Send/receive with full blocking I/O
- Automatic retry on partial sends
- Connection state tracking

## Threading Model

### Server Threading

```
Main Thread
  ├─ Accept Thread (acceptLoop)
  │    ├─ Accepts connections
  │    ├─ Creates ClientSession
  │    └─ Spawns session threads
  │
  ├─ ClientSession Thread 1
  │    └─ Processes requests
  │
  ├─ ClientSession Thread 2
  │    └─ Processes requests
  │
  └─ Command Input Thread (main)
       └─ Handles server commands
```

**Thread Safety**:
- `sessions_` vector protected by `sessionsMutex_`
- Atomic `running_` flag for shutdown coordination
- Clean thread join on shutdown

## Data Flow

### File Upload (PUT)
```
Client                              Server
  │                                   │
  ├─ Open file ─────────────────────→│
  ├─ Send CMD_PUT ──────────────────→│
  ├─ Send filename ─────────────────→│
  ├─ Send file size ────────────────→│ Create output file
  ├─ Send file data (8KB chunks) ──→│ Write to disk
  ├─ ... more chunks ...  ──────────→│ ...
  └─ Complete ──────────────────────→│ Close file
```

### File Download (GET)
```
Client                              Server
  │                                   │
  ├─ Send CMD_GET ──────────────────→│
  ├─ Send filename ─────────────────→│ Open file
  │←─ Receive file size ─────────────┤
  │←─ Receive file data (8KB chunks)─┤ Read from disk
  │←─ ... more chunks ... ───────────┤ ...
  │←─ Complete ──────────────────────┤ Close file
  └─ Save to disk                     │
```

## Error Handling

### Graceful Shutdown
1. Signal handler catches SIGINT/SIGTERM
2. Set `running_ = false`
3. Close server socket (unblocks accept)
4. Stop all client sessions
5. Join/detach threads
6. Cleanup resources

### Exception Safety
- Try-catch in session threads
- RAII for socket cleanup
- Automatic session cleanup on disconnect

## Build System Architecture

### CMake Structure
```
CMakeLists.txt (root)
  ├─ libfiletransfer.a (static library)
  │    ├─ All src/**/*.cpp files
  │    └─ Include directories
  ├─ server_test (executable)
  │    └─ Link libfiletransfer
  └─ client_test (executable)
       └─ Link libfiletransfer
```

### Dependencies
- **C++17**: std::filesystem, std::string_view
- **pthread**: Multi-threading
- **POSIX sockets**: Network I/O

## Security Considerations

**Current Implementation**:
- No authentication/encryption (educational purpose)
- Local network use recommended
- File permissions preserved

**Future Enhancements**:
- SSL/TLS encryption
- User authentication
- Access control lists
- Rate limiting

## Performance Characteristics

**Buffer Size**: 8192 bytes (8KB)
**Max Connections**: 10 (configurable)
**Timeout**: 300 seconds (5 minutes)
**Binary Transfer**: Full integrity preserved

## Scalability

**Current Limits**:
- Thread-per-client model
- Suitable for ~10-100 clients
- CPU and memory bound by threads

**Improvements for Production**:
- Thread pool architecture
- epoll/kqueue for event-driven I/O
- Load balancing
- Connection pooling

```
FileTransferCore (SHARED)
    ├── PUBLIC: Threads::Threads
    ├── PRIVATE: rt (Linux)
    └── PRIVATE: stdc++fs (if needed)
```

### Source Collection Strategy

The project uses **GLOB** patterns for automatic source discovery:

```cmake
# Client Core Sources
file(GLOB CLIENT_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/Client/*.cpp"
)

# Client Main Sources
file(GLOB CLIENT_MAIN_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/Client.cpp"
)

# Server Core Sources
file(GLOB SERVER_CORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/Server/*.cpp"
)

# Utility Sources
file(GLOB_RECURSE UTILS_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/*.cpp"
)
```

**Benefits**:
- ✅ Automatic detection of new source files
- ✅ No need to manually update CMakeLists.txt when adding files
- ✅ Clean and maintainable build configuration

**Trade-offs**:
- ⚠️ Requires CMake reconfiguration when adding new files
- ⚠️ Can accidentally include unwanted files

### Include Directory Strategy

```cmake
target_include_directories(FileTransferCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
```

**PUBLIC includes**: Available to library users
**PRIVATE includes**: Only for internal implementation

## Library Linking

### Internal Linking

All components are compiled into the single `FileTransferCore` shared library. No internal linking is required.

### External Dependencies

```cmake
# Required
Threads::Threads         # POSIX threading (pthread)

# Platform-specific
rt                      # Realtime extensions (Linux)
stdc++fs                # Filesystem library (older GCC)
ws2_32                  # Winsock2 (Windows)
```

### Usage in External Projects

```cmake
find_package(FileTransferCore REQUIRED)
target_link_libraries(MyApp PRIVATE FileTransfer::FileTransferCore)
```

## Compilation Flow

1. **Configure**: CMake scans for source files using GLOB patterns
2. **Generate**: Creates build system files (Makefile, Ninja, etc.)
3. **Compile**: All `.cpp` files are compiled to object files
4. **Link**: Object files are linked into `libfiletransfer.so`
5. **Install**: Library and headers are installed to system paths

## Build Configurations

### Debug Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Release Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Build Options
```cmake
BUILD_SHARED_LIBS        # Build shared libraries (default: ON)
BUILD_EXAMPLES           # Build example applications (default: OFF)
BUILD_TESTS              # Build test suite (default: OFF)
ENABLE_STRICT_WARNINGS   # Enable strict compiler warnings (default: ON)
```

## Library Versioning

```cmake
LIBRARY_VERSION_MAJOR = 1
LIBRARY_VERSION_MINOR = 0
LIBRARY_VERSION_PATCH = 0
```

**Shared library naming**:
- `libfiletransfer.so` (symlink to latest)
- `libfiletransfer.so.1` (symlink to patch version)
- `libfiletransfer.so.1.0.0` (actual library)

## Installation Structure

After `make install`:

```
/usr/local/
├── lib/
│   ├── libfiletransfer.so -> libfiletransfer.so.1
│   ├── libfiletransfer.so.1 -> libfiletransfer.so.1.0.0
│   ├── libfiletransfer.so.1.0.0
│   └── cmake/
│       └── FileTransferCore/
│           ├── FileTransferCoreConfig.cmake
│           ├── FileTransferCoreConfigVersion.cmake
│           └── FileTransferCoreTargets.cmake
└── include/
    ├── core/
    │   ├── Client/
    │   │   └── *.h
    │   └── Server/
    │       └── *.h
    └── utils/
        └── *.h
```

## Design Patterns

### 1. Interface-Based Design
```cpp
class Client {          // Abstract interface
    virtual bool connect(...) = 0;
    virtual bool uploadFile(...) = 0;
    // ...
};

class Client : public Client {  // Concrete implementation
    // Implementation details
};
```

### 2. Component Separation
- Each component (Socket, Protocol, Metrics) is in its own file
- Clear responsibility boundaries
- Easy to test and maintain

### 3. PIMPL (Pointer to Implementation) - Potential
Could be used to hide implementation details and reduce compile-time dependencies

## Thread Safety

The library components should be designed with thread safety in mind:
- Client instances are not thread-safe (one client per thread)
- Server should handle multiple client connections concurrently
- Metrics collection uses appropriate synchronization

## Memory Management

- RAII (Resource Acquisition Is Initialization)
- Smart pointers (std::unique_ptr, std::shared_ptr)
- No manual memory management

## Error Handling Strategy

1. **Return codes**: Boolean for simple success/failure
2. **Exceptions**: For exceptional conditions
3. **Logging**: Detailed error information for debugging

## Performance Considerations

- **Chunked Transfer**: Files transferred in configurable chunks (64KB default)
- **Buffering**: Socket operations use appropriate buffer sizes
- **Metrics**: Optional performance tracking with minimal overhead

## Future Enhancements

1. **Separate Client and Server Libraries**: Split into `FileTransferClient` and `FileTransferServer`
2. **Static Library Option**: Add option for static linking
3. **Plugin Architecture**: Dynamic loading of protocol implementations
4. **Encryption Support**: TLS/SSL for secure communication
5. **Compression**: On-the-fly file compression

## Testing Strategy

### Unit Tests
- Test individual components in isolation
- Mock dependencies using interfaces

### Integration Tests
- Test client-server interaction
- Test file transfer scenarios

### System Tests
- End-to-end testing
- Performance benchmarks

## Documentation Standards

- **Header Comments**: Doxygen-style documentation
- **Code Comments**: Explain "why", not "what"
- **README Files**: Component-level documentation
- **Architecture Docs**: This file

## Contributing Guidelines

When adding new components:
1. Place source files in appropriate `src/` subdirectory
2. Place headers in corresponding `include/` subdirectory
3. Update this architecture document
4. Add unit tests
5. Run CMake reconfiguration to pick up new files

## Troubleshooting

### Common Build Issues

**Issue**: `filesystem` not found
**Solution**: Ensure GCC 8+ or Clang 7+, or install `stdc++fs`

**Issue**: Threading errors
**Solution**: Ensure pthread is available on system

**Issue**: Source files not found
**Solution**: Run `cmake ..` again to re-scan for files

## References

- [CMake Modern Best Practices](https://cliutils.gitlab.io/modern-cmake/)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
- [POSIX Socket Programming](https://man7.org/linux/man-pages/man7/socket.7.html)
