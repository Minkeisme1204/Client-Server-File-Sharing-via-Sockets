# File Transfer GUI Applications

Qt-based graphical interface for the file transfer client and server.

## Overview

This folder contains two GUI applications built with Qt5/Qt6:
- **client_gui**: Graphical client for file operations
- **server_gui**: Graphical server management interface

Both applications use the existing `libfiletransfer.a` library.

## Prerequisites

- Qt5 (5.15+) or Qt6
- C++17 compiler
- CMake 3.16+
- libfiletransfer.a (built from parent directory)

### Installing Qt on Ubuntu/Debian:
```bash
# Qt5
sudo apt-get install qt5-default libqt5widgets5

# Qt6
sudo apt-get install qt6-base-dev
```

### Installing Qt on Fedora/RHEL:
```bash
# Qt5
sudo dnf install qt5-qtbase-devel

# Qt6
sudo dnf install qt6-qtbase-devel
```

## Building

```bash
cd app
mkdir build && cd build
cmake ..
make
```

This will create two executables:
- `client_gui`
- `server_gui`

## Usage

### Server GUI

Start the server GUI first:

```bash
./build/server_gui
```

**Features:**
- Configure port and shared directory
- Start/Stop server
- View active clients in real-time
- Monitor server metrics (connections, files transferred, throughput)
- Execute commands (clients, metrics, verbose)
- Change shared directory while running
- Export metrics to CSV

**Steps:**
1. Set port (default: 9000)
2. Set shared directory (default: ./shared)
3. Click "Start Server"
4. Monitor clients in "Active Clients" tab
5. View statistics in "Server Metrics" tab
6. Use action buttons for quick operations

### Client GUI

Start the client GUI after server is running:

```bash
./build/client_gui
```

**Features:**
- Connect to server with IP and port
- List files on server
- Download files with progress indication
- Upload files with file browser
- View transfer log with color-coded messages
- Monitor client metrics (bytes sent/received, files transferred)
- Export metrics to CSV

**Steps:**
1. Enter server IP and port (e.g., 127.0.0.1:9000)
2. Click "Connect"
3. Click "List Files" to see available files
4. Select file and click "Download File"
5. Click "Upload File" to send a file
6. View metrics in the metrics panel
7. Check transfer log in the "Transfer Log" tab

## Features

### Client Window
- **Connection Status**: Green (connected) / Red (disconnected)
- **Server Files Tab**: Shows list of available files on server
- **Transfer Log Tab**: Color-coded operation history
  - 🔵 Blue: Information
  - 🟢 Green: Success
  - 🔴 Red: Errors
- **Metrics Panel**: Real-time statistics
  - Bytes Sent/Received
  - Files Uploaded/Downloaded
  - Average Throughput
- **Quick Actions**: Connect, Disconnect, List, Download, Upload, Metrics, Export

### Server Window
- **Server Status**: Running (green) / Stopped (red)
- **Configuration**: Port and shared directory setup
- **Active Clients Tab**: Live list of connected clients with IP:PORT
- **Server Metrics Tab**: Comprehensive statistics
  - Active Clients
  - Total Connections
  - Files Sent/Received
  - Total Data Transferred
- **Commands**: Execute server commands (clients, metrics, verbose)
- **Quick Actions**: Metrics, Clients, Change Dir, Verbose, Export CSV, Stop

## File Structure

```
app/
├── CMakeLists.txt          # Qt build configuration
├── README.md               # This file
├── client/
│   ├── main.cpp            # Client GUI entry point
│   ├── ClientWindow.h      # Client window header
│   └── ClientWindow.cpp    # Client window implementation
└── server/
    ├── main.cpp            # Server GUI entry point
    ├── ServerWindow.h      # Server window header
    └── ServerWindow.cpp    # Server window implementation
```

## API Usage

The GUI applications use the same API as the CLI tests:

### Client API
```cpp
Client client;
client.connect(host, port);
client.listFiles();
client.getFile(remote_file, local_file);
client.putFile(local_file, remote_file);
client.getMetrics();
```

### Server API
```cpp
Server server;
server.start(port, shared_dir);
server.run();  // In separate thread
server.getActiveClients();
server.getMetrics();
server.stop();
```

## Troubleshooting

### Qt not found
```bash
# Set Qt path manually
export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt5
cmake ..
```

### Library not found
Make sure libfiletransfer.a exists:
```bash
cd ..
mkdir build && cd build
cmake ..
make
```

### Port already in use
Change the port in server GUI configuration before starting.

### Connection refused
- Ensure server is running
- Check firewall settings
- Verify IP address and port are correct

## Screenshots

The GUI design matches the provided reference screenshot with:
- Modern Qt5/Qt6 widgets
- Color-coded status indicators
- Tabbed interface for organization
- Real-time metrics updates
- Professional appearance

## License

Same license as the parent project (MIT License).
