# File Transfer System - Quick Start Guide

## 🚀 Quick Start (5 Minutes)

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential g++ cmake

# Fedora/RHEL
sudo dnf install gcc-c++ cmake make

# macOS
xcode-select --install
brew install cmake
```

### 1. Build Project
```bash
cd Client-Server-File-Sharing-via-Sockets
cd build
cmake ..
make
```

**Output:** `server_test` and `client_test` executables in `build/` directory

### 2. Start Server (Terminal 1)
```bash
# From project root
./build/server_test
```

**Default configuration:**
- Port: 8080
- Shared directory: `./shared`
- Max connections: 10

**Server Commands:**
```
Server> metrics     # View server statistics
Server> clients     # List active connections
Server> help        # Show all commands
Server> quit        # Stop server
```

### 3. Start Client (Terminal 2)
```bash
# Auto-connect to server
./build/client_test 127.0.0.1 8080
```

**Client Commands:**
```
Client> list                    # List files on server
Client> get test_file.txt       # Download file
Client> put myfile.txt          # Upload file
Client> metrics                 # View statistics
Client> help                    # Show all commands
Client> quit                    # Exit client
```

## 📋 Common Tasks

### Upload a File
```bash
Client> put /path/to/myfile.txt
# File uploaded to server's shared/ directory
```

### Download a File
```bash
Client> list                    # See available files
Client> get filename.txt        # Download to current directory
Client> get filename.txt /tmp   # Download to specific directory
```

### Transfer Binary Files (Audio, Video, Images)
```bash
Client> put song.mp3           # Upload audio
Client> put video.mp4          # Upload video
Client> get image.jpg          # Download image
# All binary files are fully supported!
```

## 🔧 Configuration

### Server Options
```bash
./build/server_test <port> [shared_dir]

# Examples:
./build/server_test 9000                    # Port 9000
./build/server_test 9000 /home/user/files   # Custom directory
```

### Client Options
```bash
./build/client_test [server_ip] [port]

# Examples:
./build/client_test                      # Manual connect
./build/client_test 127.0.0.1 8080      # Auto-connect localhost
./build/client_test 192.168.1.100 9000  # Auto-connect remote
```

## 🧪 Quick Tests

```bash
# Run automated tests
./quick_start.sh      # Display quick guide
./test_list.sh        # Test LIST command
./test_put.sh         # Test PUT upload
./test_get.sh         # Test GET download
```

## ❓ Troubleshooting

### Server won't start
```bash
# Check if port is in use
netstat -tuln | grep 8080

# Use different port
./build/server_test 9000
```

### Client can't connect
- Ensure server is running
- Check IP address and port
- Verify firewall settings

### File not found
- Use `list` command to see available files
- Check file exists in shared directory
- Verify file permissions

## 📖 More Information

- [TEST_USAGE.md](TEST_USAGE.md) - Detailed usage guide
- [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) - Technical details
- [BUILD.md](BUILD.md) - Build options

## 🎯 Example Session

```bash
# Terminal 1 - Server
$ ./build/server_test
[SUCCESS] Server started on port 8080
Server> 

# Terminal 2 - Client
$ ./build/client_test 127.0.0.1 8080
[SUCCESS] Connected to server!

Client> list
Files on server (3):
  1. test_file.txt
  2. welcome.txt
  3. config.txt

Client> get test_file.txt
[SUCCESS] File downloaded!

Client> put myfile.txt
[SUCCESS] File uploaded!

Client> metrics
=== Client Metrics ===
Files transferred: 2
Bytes sent: 1024
...

Client> quit
[CLIENT] Goodbye!
```

That's it! You're ready to transfer files! 🎉

### Running the Client

#### Interactive Mode
```bash
./build/bin/client_app
```

This launches an interactive menu:
```
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

## 📝 Basic Operations

### 1. Connect to Server
```
Choose option 1
Enter IP: 127.0.0.1
Enter port: 8080
```

### 2. Upload a File
```
Choose option 2
Enter file path: /path/to/your/file.txt
```

Progress will be displayed:
```
[████████████████████▶                    ] 50.0% (512000 bytes)
```

### 3. Download a File
```
Choose option 3
Enter filename: remote_file.txt
Enter save directory: ./downloads
```

### 4. Check Connection
```
Choose option 6 (Ping)
✅ Server is alive! RTT: 5 ms
```

## 💻 Programming with the Client Library

### Basic Example
```cpp
#include "FilesTransfer/Client/Client.h"

using namespace FileTransfer;

int main() {
    // Create client
    auto client = createClient();
    
    // Connect to server
    if (!client->connect("127.0.0.1", 8080)) {
        std::cerr << "Connection failed!" << std::endl;
        return 1;
    }
    
    // Upload a file
    if (client->uploadFile("myfile.txt")) {
        std::cout << "Upload successful!" << std::endl;
    }
    
    // Disconnect
    client->disconnect();
    
    return 0;
}
```

### With Progress Tracking
```cpp
#include "FilesTransfer/Client/Client.h"
#include <iostream>
#include <iomanip>

using namespace FileTransfer;

void showProgress(double progress, uint64_t bytes) {
    std::cout << "\rProgress: " << std::fixed << std::setprecision(1)
              << (progress * 100.0) << "% (" << bytes << " bytes)" 
              << std::flush;
}

int main() {
    auto client = createClient();
    
    // Set progress callback
    client->setProgressCallback(showProgress);
    
    // Set error callback
    client->setErrorCallback([](const std::string& error) {
        std::cerr << "\nError: " << error << std::endl;
    });
    
    client->connect("192.168.1.100", 8080);
    client->uploadFile("large_file.bin");
    
    std::cout << "\n" << client->getMetricsSummary() << std::endl;
    
    return 0;
}
```

### Compile Your Program
```bash
g++ -std=c++17 -I"include " myprogram.cpp \
    -L./build/bin -lfiletransfer_client -lclient_core \
    -lpthread -o myprogram
```

## 🔧 Configuration Options

### Set Chunk Size
```cpp
client->setChunkSize(128 * 1024);  // 128KB chunks
```

**Guidelines**:
- Small files: 32-64KB
- Medium files: 64-256KB
- Large files: 256KB-1MB

### Enable Verbose Mode
```cpp
client->setVerbose(true);
```

This will print detailed logs:
```
[INFO] Connecting to 127.0.0.1:8080
[INFO] Connected successfully
[INFO] Uploading file: test.txt (1024 bytes)
[INFO] Upload completed successfully
```

## 📊 Monitoring Performance

### View Real-time Metrics
```cpp
// During transfer
auto metrics = client->getTransferMetrics();
std::cout << "Speed: " << metrics.bytesPerSecond << " B/s" << std::endl;
std::cout << "Progress: " << (metrics.progress * 100) << "%" << std::endl;
std::cout << "ETA: " << metrics.estimatedTimeMs << " ms" << std::endl;

// After transfer
std::cout << client->getMetricsSummary() << std::endl;
```

### Connection Statistics
```cpp
auto connMetrics = client->getConnectionMetrics();
std::cout << "Packets sent: " << connMetrics.packetsSent << std::endl;
std::cout << "Packets received: " << connMetrics.packetsReceived << std::endl;
std::cout << "RTT: " << connMetrics.roundTripTimeMs << " ms" << std::endl;
```

## 🐛 Troubleshooting

### Connection Refused
```
❌ Connection failed: Connection refused
```
**Solution**: Ensure server is running and accessible

### File Not Found
```
❌ Upload failed: File not found: myfile.txt
```
**Solution**: Check file path is correct and file exists

### Permission Denied
```
❌ Failed to create file: Permission denied
```
**Solution**: Ensure you have write permissions in the target directory

### Enable Debug Mode
```bash
# Build with debug symbols
make -f Makefile_new debug

# Run with verbose mode
./build/bin/client_app
# Then choose option 8 to toggle verbose mode
```

## 📁 Project Structure
```
Client-Server-File-Sharing-via-Sockets/
├── include /FilesTransfer/Client/
│   ├── Client.h          # Interface definition
│   ├── Client.h           # Client implementation
│   ├── metrics.h          # Metrics system
│   └── core/
│       ├── client_socket.h
│       ├── client_protocol.h
│       └── client_metrics.h
├── src/Client/
│   ├── Client.cpp         # Implementation
│   ├── metrics.cpp
│   ├── client_app.cpp     # Example application
│   └── core/
│       ├── client_socket.cpp
│       ├── client_protocol.cpp
│       └── client_metrics.cpp
├── CMakeLists_new.txt
├── Makefile_new
└── DESIGN.md
```

## 🔗 Next Steps

1. **Read the Design Document**: [DESIGN.md](DESIGN.md)
2. **Explore the API**: Check header files for full API reference
3. **Build Your Application**: Use the client library in your projects
4. **Contribute**: Submit improvements and bug fixes

## 💡 Tips

1. **Always check connection status** before operations:
   ```cpp
   if (!client->isConnected()) {
       client->connect(ip, port);
   }
   ```

2. **Use callbacks for better UX**:
   ```cpp
   client->setProgressCallback(displayProgress);
   client->setErrorCallback(handleError);
   ```

3. **Handle errors gracefully**:
   ```cpp
   if (!client->uploadFile(path)) {
       std::cerr << client->getLastError() << std::endl;
       // Retry or fallback logic
   }
   ```

4. **Monitor metrics for optimization**:
   ```cpp
   auto metrics = client->getTransferMetrics();
   if (metrics.bytesPerSecond < 1000) {
       // Adjust chunk size or check network
   }
   ```

## 📞 Support

- Create an issue on GitHub
- Check existing documentation
- Review code examples in `src/Client/`

## ✅ Checklist for First Run

- [ ] Prerequisites installed
- [ ] Project built successfully
- [ ] Server is running
- [ ] Client can connect
- [ ] Test file upload works
- [ ] Test file download works
- [ ] Metrics are displayed correctly

Enjoy using the File Transfer Client! 🎉
