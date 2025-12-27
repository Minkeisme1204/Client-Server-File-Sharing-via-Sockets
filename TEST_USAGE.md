# Hướng dẫn sử dụng Client-Server File Transfer Test

## Build Project

```bash
cd build
cmake ..
make
```

Sau khi build thành công, bạn sẽ có 2 file thực thi:
- `server_test` - Chương trình server
- `client_test` - Chương trình client

## Chạy Server

### Cách 1: Sử dụng cấu hình mặc định
```bash
./build/server_test
```

Server sẽ chạy với:
- Port: 8080
- Thư mục chia sẻ: ./shared

### Cách 2: Tùy chỉnh port và thư mục
```bash
./build/server_test <port> [shared_directory]
```

Ví dụ:
```bash
./build/server_test 9000 ./my_files
```

### Các lệnh trong Server

Khi server đang chạy, bạn có thể nhập các lệnh sau:

- `metrics` - Hiển thị thống kê server
- `clients` - Xem danh sách client đang kết nối
- `reset` - Reset thống kê
- `export` - Xuất thống kê ra file CSV
- `dir` - Thay đổi thư mục chia sẻ
- `verbose` - Bật/tắt chế độ verbose logging
- `help` - Hiển thị menu trợ giúp
- `quit` hoặc `exit` - Dừng server

## Chạy Client

### Cách 1: Chạy và kết nối sau
```bash
./build/client_test
```

Sau đó sử dụng lệnh `connect` để kết nối đến server:
```
Client> connect 127.0.0.1 8080
```

### Cách 2: Tự động kết nối khi khởi động
```bash
./build/client_test <server_ip> <port>
```

Ví dụ:
```bash
./build/client_test 127.0.0.1 8080
```

### Các lệnh trong Client

- `connect <ip> <port>` - Kết nối đến server
- `disconnect` - Ngắt kết nối
- `list` - Xem danh sách file trên server
- `get <filename>` - Tải file từ server
- `get <filename> <save_dir>` - Tải file và lưu vào thư mục chỉ định
- `put <filepath>` - Upload file lên server
- `metrics` - Xem thống kê client
- `reset` - Reset thống kê
- `export` - Xuất thống kê ra CSV
- `status` - Xem trạng thái kết nối
- `help` - Hiển thị menu trợ giúp
- `quit` hoặc `exit` - Thoát chương trình

## Ví dụ Demo đầy đủ

### Terminal 1 - Chạy Server
```bash
cd /home/minkeisrtx5090/Desktop/Workplace/HUST/06_Computer_Network/Client-Server-File-Sharing-via-Sockets
./build/server_test
```

Server sẽ hiển thị:
```
╔═══════════════════════════════════════════════════════════╗
║          FILE TRANSFER SERVER - TEST APPLICATION          ║
╚═══════════════════════════════════════════════════════════╝

[SERVER] Starting file transfer server...
[CONFIG] Port: 8080
[CONFIG] Shared Directory: ./shared
[CONFIG] Verbose Mode: ON

[SUCCESS] Server started successfully!
[INFO] Server is listening on port 8080
[INFO] Shared directory: ./shared

Server> 
```

### Terminal 2 - Chạy Client
```bash
cd /home/minkeisrtx5090/Desktop/Workplace/HUST/06_Computer_Network/Client-Server-File-Sharing-via-Sockets
./build/client_test 127.0.0.1 8080
```

Client sẽ tự động kết nối và hiển thị:
```
╔═══════════════════════════════════════════════════════════╗
║          FILE TRANSFER CLIENT - TEST APPLICATION          ║
╚═══════════════════════════════════════════════════════════╝

[INFO] Auto-connecting to 127.0.0.1:8080...
[SUCCESS] Connected to server!

Client> 
```

### Demo các thao tác

#### 1. Xem danh sách file trên server
```
Client> list
```

#### 2. Tải file test_file.txt từ server
```
Client> get test_file.txt
```

#### 3. Upload một file lên server
Trước tiên tạo một file mới:
```bash
echo "Hello from client!" > my_upload.txt
```

Sau đó upload:
```
Client> put ./my_upload.txt
```

#### 4. Xem thống kê
```
Client> metrics
```

#### 5. Ngắt kết nối
```
Client> disconnect
```

#### 6. Kết nối lại
```
Client> connect 127.0.0.1 8080
```

#### 7. Thoát
```
Client> quit
```

## File test mẫu

Hệ thống đã tạo sẵn file `shared/test_file.txt` để bạn có thể test ngay chức năng download.

## Lưu ý

1. **Server phải chạy trước client**
2. **Thư mục shared phải tồn tại** - Server sẽ tự động tạo nếu chưa có
3. **Port phải khả dụng** - Nếu port đã được sử dụng, hãy chọn port khác
4. **Firewall** - Đảm bảo firewall không chặn kết nối
5. **Quyền truy cập** - Đảm bảo có quyền đọc/ghi thư mục shared

## Test với nhiều client

Bạn có thể mở nhiều terminal và chạy nhiều client cùng lúc:

```bash
# Terminal 3
./build/client_test 127.0.0.1 8080

# Terminal 4  
./build/client_test 127.0.0.1 8080

# Terminal 5
./build/client_test 127.0.0.1 8080
```

Server hỗ trợ tối đa 10 kết nối đồng thời (có thể thay đổi trong code).

## Tính năng nổi bật

- ✅ Giao diện terminal thân thiện
- ✅ Hỗ trợ upload/download file
- ✅ Liệt kê file trên server
- ✅ Thống kê chi tiết (metrics)
- ✅ Nhiều client đồng thời
- ✅ Tự động xử lý lỗi
- ✅ Xuất thống kê ra CSV
- ✅ Verbose logging
- ✅ Graceful shutdown (Ctrl+C)

## Troubleshooting

### Server không khởi động được
- Kiểm tra port có đang được sử dụng không: `netstat -tuln | grep 8080`
- Thử port khác: `./build/server_test 9000`

### Client không kết nối được
- Đảm bảo server đang chạy
- Kiểm tra IP và port đúng
- Kiểm tra firewall: `sudo ufw status`

### File không upload được
- Kiểm tra file có tồn tại không
- Kiểm tra quyền đọc file
- Kiểm tra dung lượng đĩa

### File không download được
- Đảm bảo file tồn tại trên server (dùng `list`)
- Kiểm tra quyền ghi thư mục đích
- Kiểm tra dung lượng đĩa
