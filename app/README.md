# File Transfer GUI Applications

Các ứng dụng GUI được xây dựng bằng Qt5 cho Client và Server.

## Cấu trúc thư mục

```
app/
├── client/              # Ứng dụng GUI cho Client
│   ├── ClientWindow.h
│   ├── ClientWindow.cpp
│   └── main.cpp
├── server/              # Ứng dụng GUI cho Server
│   ├── ServerWindow.h
│   ├── ServerWindow.cpp
│   └── main.cpp
├── common/              # Các thành phần dùng chung (nếu có)
└── CMakeLists.txt       # File cấu hình CMake
```

## Yêu cầu

- **Qt5** (Qt5 Widgets)
- **CMake** >= 3.10
- **C++17** compiler
- Thư viện `filetransfer` đã được build

## Cài đặt Qt5

### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install qt5-default qtbase5-dev
```

### Fedora/CentOS:
```bash
sudo dnf install qt5-qtbase-devel
```

### macOS:
```bash
brew install qt@5
```

## Build ứng dụng

Từ thư mục gốc của project:

```bash
cd build
cmake ..
make client_gui server_gui
```

Hoặc rebuild toàn bộ:

```bash
cd build
cmake ..
make
```

## Chạy ứng dụng

### Server GUI:
```bash
./build/server_gui
```

Server sẽ tự động khởi động trên port 8080 với shared directory là `./shared`.

### Client GUI:
```bash
./build/client_gui
```

Sau đó nhập lệnh: `connect <IP> <port>` để kết nối đến server.

## Tính năng

### Client GUI:
- ✅ Kết nối/Ngắt kết nối đến server
- ✅ Liệt kê files trên server
- ✅ Download files từ server
- ✅ Upload files lên server
- ✅ Xem metrics và history
- ✅ Export metrics ra CSV
- ✅ Giao diện command-line tích hợp
- ✅ Nút bấm nhanh cho các chức năng chính

### Server GUI:
- ✅ Khởi động/Dừng server
- ✅ Xem danh sách clients đang kết nối
- ✅ Hiển thị server metrics
- ✅ Thay đổi shared directory
- ✅ Bật/tắt verbose logging
- ✅ Export metrics ra CSV
- ✅ Cập nhật trạng thái real-time
- ✅ Log hiển thị với timestamp

## Giao diện

Cả hai ứng dụng sử dụng dark theme với:
- Màu nền tối để giảm mỏi mắt
- Syntax highlighting cho output
- Nút bấm có màu sắc rõ ràng
- Layout responsive

## Sử dụng

### Client Commands:
- `connect <ip> <port>` - Kết nối đến server
- `disconnect` - Ngắt kết nối
- `list` - Liệt kê files
- `get <filename>` - Download file
- `put <filepath>` - Upload file
- `metrics` - Hiển thị metrics
- `history [limit]` - Hiển thị lịch sử
- `reset` - Reset metrics
- `export` - Export metrics
- `status` - Xem trạng thái
- `help` - Hiển thị help
- `quit/exit` - Thoát

### Server Commands:
- `start [port] [dir]` - Khởi động server
- `stop` - Dừng server
- `metrics` - Hiển thị metrics
- `clients` - Xem danh sách clients
- `reset` - Reset metrics
- `export` - Export metrics
- `dir` - Thay đổi shared directory
- `verbose` - Toggle verbose mode
- `help` - Hiển thị help
- `quit/exit` - Thoát

## Lưu ý

- Ứng dụng GUI sử dụng thư viện `filetransfer` có sẵn
- Không có file nào bên ngoài thư mục `app/` bị chỉnh sửa
- Server tự động khởi động khi mở ứng dụng
- Client cần connect thủ công đến server
- Tất cả các tính năng đều có thể truy cập qua cả command và button

## Troubleshooting

### Lỗi "Qt5 not found":
```bash
export CMAKE_PREFIX_PATH=/usr/local/opt/qt5  # macOS
export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt5  # Linux
```

### Lỗi khi chạy server:
- Đảm bảo port không bị sử dụng bởi process khác
- Kiểm tra shared directory có tồn tại và có quyền truy cập

### Lỗi khi connect client:
- Đảm bảo server đang chạy
- Kiểm tra IP và port đúng
- Kiểm tra firewall không chặn kết nối
