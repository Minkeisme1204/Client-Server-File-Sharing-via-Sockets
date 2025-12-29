# HƯỚNG DẪN SỬ DỤNG ỨNG DỤNG GUI

## 📋 Mục lục
1. [Cài đặt](#cài-đặt)
2. [Build ứng dụng](#build-ứng-dụng)
3. [Chạy ứng dụng](#chạy-ứng-dụng)
4. [Sử dụng Server GUI](#sử-dụng-server-gui)
5. [Sử dụng Client GUI](#sử-dụng-client-gui)
6. [Tính năng chi tiết](#tính-năng-chi-tiết)
7. [Xử lý lỗi](#xử-lý-lỗi)

## 🔧 Cài đặt

### Yêu cầu hệ thống:
- **Operating System**: Linux, macOS, hoặc Windows (với MinGW)
- **CMake**: >= 3.10
- **C++ Compiler**: g++ hoặc clang++ hỗ trợ C++17
- **Qt5**: Qt5 Widgets module
- **Threads**: pthread

### Cài đặt Qt5:

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install qt5-default qtbase5-dev build-essential cmake
```

#### Fedora/CentOS:
```bash
sudo dnf install qt5-qtbase-devel cmake gcc-c++
```

#### macOS:
```bash
brew install qt@5 cmake
export CMAKE_PREFIX_PATH=/usr/local/opt/qt5
```

## 🛠️ Build ứng dụng

### Cách 1: Sử dụng script tự động (Khuyến nghị)
```bash
./build_gui.sh
```

### Cách 2: Build thủ công
```bash
# Tạo và vào thư mục build
mkdir -p build
cd build

# Chạy CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build cả thư viện và GUI
make

# Hoặc chỉ build GUI
make client_gui server_gui
```

### Kiểm tra build thành công:
```bash
ls -la build/client_gui build/server_gui
```

Bạn sẽ thấy 2 file thực thi:
- `build/client_gui` - Ứng dụng Client
- `build/server_gui` - Ứng dụng Server

## ▶️ Chạy ứng dụng

### Khởi động Server (Bước 1):
```bash
cd build
./server_gui
```

Server sẽ tự động:
- Khởi động trên port **8080**
- Sử dụng thư mục `./shared` làm shared directory
- Hiển thị giao diện GUI với trạng thái "Running"

### Khởi động Client (Bước 2):
```bash
# Mở terminal mới
cd build
./client_gui
```

Client sẽ hiển thị giao diện GUI. Bạn cần connect đến server bằng lệnh:
```
connect 192.168.1.10 8080
```
(Thay `192.168.1.10` bằng IP của server, hoặc dùng `127.0.0.1` nếu chạy local)

## 🖥️ Sử dụng Server GUI

### Giao diện chính:

```
┌─────────────────────────────────────────────┐
│         File Transfer Server                │
├─────────────────────────────────────────────┤
│ Available Commands:                         │
│ • start, stop, metrics, clients, reset...   │
├─────────────────────────────────────────────┤
│ Server Control: IP: 192.168.1.10 Port: 8080│
├─────────────────────────────────────────────┤
│ Status: Running on port 8080                │
├─────────────────────────────────────────────┤
│ [Server Logs]                               │
│ [12:34:56] Server started on port 8080      │
│ [12:35:01] Client connected: 192.168.1.20   │
├─────────────────────────────────────────────┤
│ [Active Clients: 5] [Server Metrics]        │
│ [Change Directory]                          │
│ [Verbose: ON] [Export CSV] [Help]           │
│                                             │
│         [Stop Server]                       │
└─────────────────────────────────────────────┘
```

### Các nút chức năng:

1. **Active Clients** 
   - Hiển thị danh sách clients đang kết nối
   - Cập nhật real-time số lượng clients

2. **Server Metrics**
   - Total Connections (Tổng số kết nối)
   - Active Sessions (Phiên đang hoạt động)
   - Total Files Sent/Received (Files đã gửi/nhận)
   - Total Bytes Sent/Received (Bytes đã truyền)
   - Uptime (Thời gian hoạt động)

3. **Change Directory**
   - Thay đổi thư mục shared directory
   - Mở file dialog để chọn thư mục

4. **Verbose: ON/OFF**
   - Bật/tắt chế độ logging chi tiết
   - Toggle giữa ON và OFF

5. **Export CSV**
   - Export metrics ra file CSV
   - Tự động đặt tên theo timestamp

6. **Help**
   - Hiển thị hướng dẫn sử dụng
   - Liệt kê tất cả commands

7. **Stop Server**
   - Dừng server (có xác nhận)
   - Sau khi dừng, nút chuyển thành "Start Server"

### Commands có thể nhập:

- `start [port] [dir]` - Khởi động server
- `stop` - Dừng server
- `metrics` - Hiển thị metrics trong log
- `clients` - Liệt kê active clients
- `reset` - Reset tất cả metrics về 0
- `export` - Export metrics
- `dir` - Thay đổi shared directory
- `verbose` - Toggle verbose mode
- `help` - Hiển thị help
- `quit` / `exit` - Thoát ứng dụng

## 💻 Sử dụng Client GUI

### Giao diện chính:

```
┌─────────────────────────────────────────────┐
│         File Transfer Client                │
├─────────────────────────────────────────────┤
│ Available Commands:                         │
│ • connect <ip><port>, list, get, put...     │
├─────────────────────────────────────────────┤
│ Command: [connect 192.168.1.10 8080] [Send]│
├─────────────────────────────────────────────┤
│ Status: Connected to 192.168.1.10:8080      │
├─────────────────────────────────────────────┤
│ [Command Output]                            │
│ Command: connect 192.168.1.10 8080          │
│ ✓ Connected to 192.168.1.10:8080            │
│ - Ready for commands...                     │
├─────────────────────────────────────────────┤
│ [Disconnect] [View Files] [Download]        │
│ [Upload] [Exit]                             │
└─────────────────────────────────────────────┘
```

### Các nút chức năng:

1. **Disconnect**
   - Ngắt kết nối khỏi server
   - Chuyển về trạng thái "Not connected"

2. **View Files** (📄)
   - Gửi lệnh LIST đến server
   - Hiển thị danh sách files có sẵn

3. **Download** (⬇)
   - Mở dialog nhập tên file
   - Chọn thư mục lưu file
   - Download file từ server

4. **Upload** (⬆)
   - Mở file browser chọn file
   - Upload file lên server
   - Hiển thị kết quả

5. **Exit** (📤)
   - Ngắt kết nối và thoát ứng dụng

### Commands có thể nhập:

#### 1. Kết nối đến server:
```
connect 192.168.1.10 8080
```

#### 2. Liệt kê files trên server:
```
list
```

#### 3. Download file:
```
get filename.txt
```
(Sẽ mở dialog chọn thư mục lưu)

#### 4. Upload file:
```
put /path/to/file.txt
```

#### 5. Xem metrics:
```
metrics
```
Hiển thị:
- RTT (Round-trip time)
- Throughput (Tốc độ truyền)
- Packet Loss Rate (Tỉ lệ mất gói)
- Total Requests/Failed Requests
- Total Bytes Transferred

#### 6. Xem lịch sử:
```
history
history 50
```
(Mặc định hiển thị 20 request gần nhất)

#### 7. Reset metrics:
```
reset
```

#### 8. Export metrics:
```
export
```
(Mở dialog chọn vị trí lưu file CSV)

#### 9. Kiểm tra trạng thái:
```
status
```

#### 10. Ngắt kết nối:
```
disconnect
```

## 🎯 Tính năng chi tiết

### Auto-complete và Shortcuts:
- **Enter** trong command input = Send command
- Các nút bấm = Shortcuts cho các lệnh thường dùng
- Output có màu sắc rõ ràng:
  - 🟢 **Green**: Thành công
  - 🔴 **Red**: Lỗi
  - 🟠 **Orange**: Cảnh báo
  - 🔵 **Blue**: Thông tin
  - 🟣 **Cyan**: Metrics/Data

### Real-time Updates:
- Status label cập nhật mỗi giây
- Active clients count tự động refresh
- Log hiển thị với timestamp
- Connection status indicator

### File Operations:
- **Upload**: Hỗ trợ mọi loại file
- **Download**: Chọn thư mục lưu linh hoạt
- **List**: Hiển thị file từ server
- Progress indication (qua output text)

### Metrics Export:
- Format: CSV
- Tên file tự động: `metrics_[timestamp].csv`
- Bao gồm tất cả số liệu thống kê
- Có thể mở bằng Excel/LibreOffice

## ⚠️ Xử lý lỗi

### Lỗi: "Qt5 not found"
```bash
# Ubuntu/Debian
sudo apt-get install qt5-default qtbase5-dev

# macOS - Set path
export CMAKE_PREFIX_PATH=/usr/local/opt/qt5

# Fedora
sudo dnf install qt5-qtbase-devel
```

### Lỗi: "Port already in use"
- Đóng process đang dùng port 8080
- Hoặc chạy server trên port khác:
```
stop
start 8081 ./shared
```

### Lỗi: "Cannot connect to server"
**Nguyên nhân:**
1. Server chưa khởi động
2. IP/Port không đúng
3. Firewall chặn kết nối
4. Network không kết nối

**Giải pháp:**
1. Kiểm tra server đang chạy: `Active Clients` button
2. Thử connect lại với IP đúng
3. Kiểm tra firewall:
```bash
# Ubuntu
sudo ufw allow 8080/tcp

# CentOS
sudo firewall-cmd --add-port=8080/tcp --permanent
sudo firewall-cmd --reload
```

### Lỗi: "Failed to upload/download"
**Nguyên nhân:**
1. File không tồn tại
2. Không có quyền truy cập
3. Shared directory không đúng
4. Kết nối bị mất

**Giải pháp:**
1. Kiểm tra file tồn tại
2. Kiểm tra permissions
3. Thay đổi shared directory trên server
4. Reconnect client

### Server không khởi động:
```bash
# Kiểm tra log
# Tạo shared directory nếu chưa có
mkdir -p ./shared

# Kiểm tra quyền
chmod 755 ./shared

# Thử port khác
```

### Client không thể gửi file:
1. Kiểm tra kết nối: `status`
2. Kiểm tra file path đúng
3. Thử upload file nhỏ hơn trước

## 📚 Tài liệu bổ sung

- [README.md](app/README.md) - Thông tin build và cấu trúc
- [BUILD.md](BUILD.md) - Hướng dẫn build chi tiết
- [CLIENT_README.md](CLIENT_README.md) - API Client
- [ARCHITECTURE.md](docs/ARCHITECTURE.md) - Kiến trúc hệ thống

## 🎨 Customization

### Thay đổi theme:
Chỉnh sửa stylesheet trong `setupUI()` của `ClientWindow.cpp` hoặc `ServerWindow.cpp`

### Thay đổi port mặc định:
Chỉnh sửa trong constructor của `ServerWindow`:
```cpp
currentPort(8080)  // Đổi thành port bạn muốn
```

### Thay đổi shared directory mặc định:
```cpp
currentSharedDir("./shared")  // Đổi path
```

## 📞 Hỗ trợ

Nếu gặp vấn đề, vui lòng:
1. Kiểm tra log output
2. Đọc phần "Xử lý lỗi" ở trên
3. Kiểm tra console output (terminal)
4. Report issue với log đầy đủ
