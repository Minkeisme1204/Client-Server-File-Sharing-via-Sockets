# 🖥️ File Transfer Application - GUI Version

## 📖 Tổng quan

Đây là ứng dụng truyền file Client-Server với giao diện đồ họa (GUI) được xây dựng bằng **Qt5**. Ứng dụng sử dụng thư viện `filetransfer` đã được build sẵn và cung cấp giao diện thân thiện cho cả Server và Client.

## 🎯 Tính năng chính

### ✨ Client GUI
- 🔌 Kết nối/Ngắt kết nối đến server
- 📋 Liệt kê files trên server
- ⬇️ Download files từ server với dialog chọn thư mục
- ⬆️ Upload files lên server với file browser
- 📊 Hiển thị metrics real-time (RTT, throughput, packet loss)
- 📜 Xem lịch sử các request
- 💾 Export metrics ra CSV
- 🎨 Dark theme với syntax highlighting
- ⚡ Command-line tích hợp + Quick action buttons

### ✨ Server GUI
- 🚀 Khởi động/Dừng server tự động
- 👥 Hiển thị danh sách clients đang kết nối
- 📈 Server metrics với cập nhật real-time
- 📁 Thay đổi shared directory dễ dàng
- 🔊 Toggle verbose logging
- 💾 Export server metrics ra CSV
- ⏰ Log với timestamp
- 🎨 Dark theme professional
- 🔄 Auto-start trên port 8080

## 📦 Cấu trúc Project

```
Client-Server-File-Sharing-via-Sockets/
│
├── app/                          # ⭐ Ứng dụng GUI (MỚI)
│   ├── client/                   # Client GUI
│   │   ├── ClientWindow.h        # Header file
│   │   ├── ClientWindow.cpp      # Implementation
│   │   └── main.cpp              # Entry point
│   ├── server/                   # Server GUI
│   │   ├── ServerWindow.h        # Header file
│   │   ├── ServerWindow.cpp      # Implementation
│   │   └── main.cpp              # Entry point
│   ├── common/                   # Shared components
│   ├── CMakeLists.txt            # CMake config cho GUI
│   ├── README.md                 # Thông tin build
│   └── USAGE_GUIDE.md            # Hướng dẫn sử dụng chi tiết
│
├── include/                      # Header files của thư viện
│   ├── client.h                  # Client API
│   ├── server.h                  # Server API
│   └── core/                     # Core components
│       ├── Client/               # Client internals
│       └── Server/               # Server internals
│
├── src/                          # Source files của thư viện
│   ├── client.cpp
│   ├── server.cpp
│   └── core/
│
├── build/                        # Build output
│   ├── client_gui               # ⭐ Client GUI executable
│   ├── server_gui               # ⭐ Server GUI executable
│   └── [other builds...]
│
├── shared/                       # Shared directory cho server
│
├── CMakeLists.txt               # CMake config chính
├── build_gui.sh                 # ⭐ Script build GUI
└── [other files...]
```

## 🚀 Quick Start

### Bước 1: Cài đặt Qt5

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install qt5-default qtbase5-dev
```

#### Fedora/CentOS:
```bash
sudo dnf install qt5-qtbase-devel
```

#### macOS:
```bash
brew install qt@5
export CMAKE_PREFIX_PATH=/usr/local/opt/qt5
```

### Bước 2: Build ứng dụng

```bash
# Sử dụng script tự động
./build_gui.sh

# Hoặc build thủ công
mkdir -p build && cd build
cmake ..
make client_gui server_gui
```

### Bước 3: Chạy ứng dụng

**Terminal 1 - Server:**
```bash
cd build
./server_gui
```
Server tự động khởi động trên port 8080

**Terminal 2 - Client:**
```bash
cd build
./client_gui
```
Trong Client, nhập lệnh:
```
connect 127.0.0.1 8080
```

### Bước 4: Thử nghiệm

Trong Client GUI:
```
list                    # Xem files
get test_file.txt       # Download file
put /path/to/file.txt   # Upload file
metrics                 # Xem thống kê
```

## 📚 Tài liệu

| File | Mô tả |
|------|-------|
| [app/README.md](app/README.md) | Build instructions và cấu trúc |
| [app/USAGE_GUIDE.md](app/USAGE_GUIDE.md) | Hướng dẫn sử dụng chi tiết |
| [BUILD.md](BUILD.md) | Build instructions cho thư viện |
| [CLIENT_README.md](CLIENT_README.md) | Client API documentation |
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | System architecture |
| [QUICKSTART.md](QUICKSTART.md) | Quick start guide |

## 🎨 Screenshots

### Client GUI
```
┌──────────────────────────────────────────────┐
│         File Transfer Client                 │
├──────────────────────────────────────────────┤
│ Available Commands:                          │
│ • connect <ip><port> - Connect to server     │
│ • disconnect - Disconnect from server        │
│ • list - List files on server                │
│ • get <filename> - Download file             │
│ • put <filepath> - Upload file               │
│ • metrics - Display client metrics           │
│ ...                                          │
├──────────────────────────────────────────────┤
│ Command: [connect 192.168.1.10 8080] [Send] │
├──────────────────────────────────────────────┤
│ - Status: Connected to 192.168.1.10:8080     │
├──────────────────────────────────────────────┤
│ Output:                                      │
│ Command: connect 192.168.1.10 8080           │
│ ✓ Connected to 192.168.1.10:8080             │
│ - Ready for commands...                      │
├──────────────────────────────────────────────┤
│ [Disconnect] [📄 View Files] [⬇ Download]    │
│ [⬆ Upload] [📤 Exit]                         │
└──────────────────────────────────────────────┘
```

### Server GUI
```
┌──────────────────────────────────────────────┐
│         File Transfer Server                 │
├──────────────────────────────────────────────┤
│ Server Control: IP: 192.168.1.10 Port: 8080 │
│ - Status: Running on port 8080               │
├──────────────────────────────────────────────┤
│ Logs:                                        │
│ [12:34:56] Server started on port 8080       │
│ [12:35:01] Client connected: 192.168.1.20    │
│ [12:35:15] File received: document.pdf       │
├──────────────────────────────────────────────┤
│ [Active Clients: 5] [Server Metrics]         │
│ [🗂 Change Directory]                         │
│ [Verbose: ON] [▶ Export CSV] [❓ Help]       │
│                                              │
│              [Stop Server]                   │
└──────────────────────────────────────────────┘
```

## 🎓 Sử dụng nâng cao

### Export Metrics
Cả Client và Server đều hỗ trợ export metrics ra file CSV:

**Client:**
```
export
```
→ Chọn vị trí lưu file (ví dụ: `client_metrics_1234567890.csv`)

**Server:**
Click nút "Export CSV" hoặc nhập:
```
export
```

### Verbose Logging
Server hỗ trợ verbose mode để xem log chi tiết:

Click nút "Verbose: OFF" → "Verbose: ON"

Hoặc nhập lệnh:
```
verbose
```

### Thay đổi Shared Directory
Thay đổi thư mục chia sẻ của server:

Click nút "Change Directory" → Chọn thư mục mới

Hoặc nhập lệnh:
```
dir
```

### Xem Active Clients
Xem danh sách clients đang kết nối:

Click nút "Active Clients: X" hoặc nhập:
```
clients
```

## 🔧 Troubleshooting

### Qt5 không tìm thấy
```bash
# Set Qt5 path
export CMAKE_PREFIX_PATH=/usr/local/opt/qt5  # macOS
export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt5  # Linux

# Rebuild
cd build
cmake ..
make client_gui server_gui
```

### Port đã được sử dụng
```bash
# Tìm process đang dùng port 8080
sudo lsof -i :8080
sudo netstat -tulpn | grep 8080

# Kill process
sudo kill -9 <PID>

# Hoặc dùng port khác
# Trong Server GUI: stop → start 8081 ./shared
```

### Không thể kết nối
1. ✅ Kiểm tra server đang chạy
2. ✅ Kiểm tra IP/Port đúng
3. ✅ Kiểm tra firewall:
```bash
sudo ufw allow 8080/tcp  # Ubuntu
sudo firewall-cmd --add-port=8080/tcp --permanent  # CentOS
```

### File không upload/download được
1. ✅ Kiểm tra quyền truy cập file
2. ✅ Kiểm tra shared directory tồn tại
3. ✅ Kiểm tra kết nối: `status`
4. ✅ Xem server logs

## 🌟 Tính năng đặc biệt

### 1. Dark Theme
Cả Client và Server đều sử dụng dark theme:
- Giảm mỏi mắt khi dùng lâu
- Highlighting cho output
- Professional appearance

### 2. Real-time Updates
- Status cập nhật mỗi giây
- Active clients count tự động
- Metrics refresh liên tục

### 3. User-Friendly
- Command help luôn hiển thị
- Quick action buttons
- File dialogs cho browse
- Confirmation dialogs
- Error messages rõ ràng

### 4. Metrics & Analytics
**Client Metrics:**
- RTT (Round-trip time)
- Throughput (kbps)
- Packet loss rate
- Total requests/bytes
- Request history

**Server Metrics:**
- Total connections
- Active sessions
- Files sent/received
- Bytes transferred
- Uptime

### 5. CSV Export
Export metrics để phân tích:
- Timestamp
- All metrics values
- Excel/LibreOffice compatible
- Auto-generated filenames

## 📝 Lưu ý quan trọng

### ⚠️ Không chỉnh sửa thư viện gốc
Ứng dụng GUI trong thư mục `app/` **CHỈ SỬ DỤNG** thư viện `filetransfer` có sẵn, **KHÔNG CHỈNH SỬA** các file bên ngoài thư mục `app/`.

### ✅ Đã thêm vào CMake
File `CMakeLists.txt` gốc đã được cập nhật để thêm:
```cmake
add_subdirectory(app)
```

### 🔗 Linking
GUI applications link với thư viện:
```cmake
target_link_libraries(client_gui
    Qt5::Widgets
    filetransfer
    pthread
)
```

## 🎯 Testing

### Test Client GUI:
1. Khởi động server_gui
2. Khởi động client_gui
3. Connect: `connect 127.0.0.1 8080`
4. List files: `list`
5. Upload file: Click "Upload" button
6. Download file: Click "Download" button
7. Check metrics: `metrics`

### Test Server GUI:
1. Khởi động server_gui (auto-start)
2. Click "Active Clients" → Xem danh sách
3. Click "Server Metrics" → Xem thống kê
4. Click "Change Directory" → Thay đổi folder
5. Click "Export CSV" → Export metrics
6. Click "Stop Server" → Dừng và restart

## 💡 Tips & Tricks

1. **Shortcut Commands**: Sử dụng nút bấm thay vì gõ lệnh để nhanh hơn
2. **File Browser**: Dùng "Upload" button để browse file thay vì nhập path
3. **CSV Analysis**: Export metrics và phân tích trong Excel
4. **Multiple Clients**: Mở nhiều client_gui để test concurrent connections
5. **Verbose Mode**: Bật verbose để debug
6. **Quick Reconnect**: Nếu mất kết nối, chỉ cần `connect` lại

## 🏆 Best Practices

1. **Luôn start Server trước Client**
2. **Kiểm tra Status trước khi thao tác**
3. **Export metrics định kỳ để tracking**
4. **Sử dụng verbose khi cần debug**
5. **Disconnect trước khi thoát Client**
6. **Stop Server gracefully (click Stop button)**

## 📞 Support

Nếu gặp vấn đề:
1. 📖 Đọc [USAGE_GUIDE.md](app/USAGE_GUIDE.md)
2. 🔍 Kiểm tra console logs
3. 📝 Check error messages trong GUI
4. 🐛 Debug với verbose mode
5. 📊 Xem metrics để tìm bottleneck

## 🔄 Updates

Version: 1.0.0
- ✅ Client GUI với đầy đủ tính năng
- ✅ Server GUI với management interface
- ✅ Dark theme
- ✅ Real-time updates
- ✅ Metrics & CSV export
- ✅ File operations với dialogs
- ✅ Complete documentation

---

**Chúc bạn sử dụng ứng dụng vui vẻ! 🎉**
