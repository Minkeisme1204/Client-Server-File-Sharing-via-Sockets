# Hướng Dẫn Sử Dụng test_metrics.sh

## Mô tả
Script tự động test và ghi log metrics (RTT, throughput, latency, packet loss) cho các file transfer operations (GET/PUT). Mỗi file được test 5 lần và tính trung bình.

## Các Tham Số Cấu Hình

### 1. TEST_FOLDER
```bash
TEST_FOLDER="./test_files"
```
- **Mô tả**: Thư mục chứa các file cần test
- **Mặc định**: `./test_files`
- **Ví dụ**:
  ```bash
  TEST_FOLDER="./my_test_data"           # Thư mục khác
  TEST_FOLDER="/home/user/Documents"     # Đường dẫn tuyệt đối
  ```

### 2. SERVER_IP
```bash
SERVER_IP="127.0.0.1"
```
- **Mô tả**: Địa chỉ IP của server
- **Mặc định**: `127.0.0.1` (localhost)
- **Ví dụ**:
  ```bash
  SERVER_IP="192.168.1.100"              # Mạng LAN
  SERVER_IP="100.64.1.5"                 # Tailscale
  ```

### 3. SERVER_PORT
```bash
SERVER_PORT="9000"
```
- **Mô tả**: Port của server
- **Mặc định**: `9000`
- **Ví dụ**:
  ```bash
  SERVER_PORT="8080"
  SERVER_PORT="12345"
  ```

### 4. OUTPUT_CSV
```bash
OUTPUT_CSV="metrics_test_$(date +%s).csv"
```
- **Mô tả**: Tên file CSV output (tự động thêm timestamp)
- **Mặc định**: `metrics_test_<timestamp>.csv`
- **Ví dụ**:
  ```bash
  OUTPUT_CSV="results.csv"                    # Tên cố định
  OUTPUT_CSV="test_$(date +%Y%m%d).csv"      # Theo ngày
  OUTPUT_CSV="./results/metrics.csv"          # Thư mục khác
  ```

### 5. ITERATIONS
```bash
ITERATIONS=5
```
- **Mô tả**: Số lần test mỗi file (để tính trung bình)
- **Mặc định**: `5`
- **Khuyến nghị**: 3-10 lần
- **Ví dụ**:
  ```bash
  ITERATIONS=3      # Test nhanh
  ITERATIONS=10     # Test chính xác hơn
  ITERATIONS=1      # Test một lần (không khuyến nghị)
  ```

## Cách Sử Dụng

### Bước 1: Chuẩn bị
```bash
# Tạo thư mục test files nếu chưa có
mkdir -p test_files

# Copy các file cần test vào
cp /path/to/your/files/* test_files/

# Kiểm tra files
ls -lh test_files/
```

### Bước 2: Chỉnh sửa tham số
```bash
nano test_metrics.sh
```
Sửa các dòng từ line 7-11:
```bash
TEST_FOLDER="./test_files"          # Thay đổi nếu cần
SERVER_IP="127.0.0.1"               # IP server của bạn
SERVER_PORT="9000"                  # Port server của bạn
OUTPUT_CSV="metrics_test_$(date +%s).csv"
ITERATIONS=5                         # Số lần test
```

### Bước 3: Chạy Server
```bash
# Terminal 1: Start server
./build/simp_server 9000
```

### Bước 4: Chạy Test
```bash
# Terminal 2: Run test script
chmod +x test_metrics.sh
./test_metrics.sh
```

## Cấu Trúc File CSV Output

```csv
STT,File Name,File Size (bytes),Operation,Packets,Throughput (Mbps),RTT (ms),Transfer Latency (ms),Packet Loss Rate (%)
1,test1.jpg,1048576,PUT,16,85.23,12.45,150.32,0.00
2,test1.jpg,1048576,GET,16,90.15,11.82,145.67,0.00
3,test2.pdf,2097152,PUT,32,78.90,15.23,200.45,0.50
4,test2.pdf,2097152,GET,32,82.34,14.56,195.12,0.00
```

### Giải thích các cột:
- **STT**: Số thứ tự
- **File Name**: Tên file
- **File Size**: Kích thước file (bytes)
- **Operation**: PUT (upload) hoặc GET (download)
- **Packets**: Số gói tin (file_size / 64KB)
- **Throughput**: Tốc độ trung bình (Mbps)
- **RTT**: Round Trip Time trung bình (ms)
- **Transfer Latency**: Thời gian transfer trung bình (ms)
- **Packet Loss Rate**: Tỷ lệ mất gói (%)

## Ví Dụ Cấu Hình

### Test cục bộ (localhost)
```bash
TEST_FOLDER="./test_files"
SERVER_IP="127.0.0.1"
SERVER_PORT="9000"
ITERATIONS=5
```

### Test qua mạng LAN
```bash
TEST_FOLDER="./test_files"
SERVER_IP="192.168.1.100"
SERVER_PORT="9000"
ITERATIONS=10                    # Nhiều lần hơn để đo chính xác
```

### Test với Tailscale
```bash
TEST_FOLDER="./test_files"
SERVER_IP="100.64.1.5"          # Tailscale IP
SERVER_PORT="9000"
ITERATIONS=7
```

### Test nhanh với ít file
```bash
TEST_FOLDER="./quick_test"
SERVER_IP="127.0.0.1"
SERVER_PORT="9000"
ITERATIONS=3                     # 3 lần cho nhanh
```

## Lưu Ý

### 1. Yêu cầu
- Server phải đang chạy trước khi test
- Client executable phải tồn tại: `./build/simp_client`
- Thư mục TEST_FOLDER phải có ít nhất 1 file

### 2. Hiệu suất
- ITERATIONS càng nhiều = chính xác hơn nhưng lâu hơn
- File lớn test lâu hơn file nhỏ
- Mỗi iteration có delay 0.5s để tránh overload

### 3. Troubleshooting

**Lỗi: "Client executable not found"**
```bash
# Build client trước
cmake --build build
```

**Lỗi: "No files found in test_files"**
```bash
# Kiểm tra thư mục
ls -la test_files/
```

**Lỗi: Connection refused**
```bash
# Kiểm tra server đang chạy
ps aux | grep simp_server

# Start server
./build/simp_server 9000
```

## Phân Tích Kết Quả

### Xem file CSV
```bash
# Xem trực tiếp
cat metrics_test_*.csv

# Mở với Excel/LibreOffice
libreoffice metrics_test_*.csv

# Parse với Python
python3 -c "import pandas as pd; df = pd.read_csv('metrics_test_*.csv'); print(df.describe())"
```

### So sánh PUT vs GET
```bash
# Filter PUT operations
grep ",PUT," metrics_test_*.csv

# Filter GET operations  
grep ",GET," metrics_test_*.csv
```

### Tính throughput trung bình
```bash
# Throughput trung bình của tất cả operations
awk -F',' 'NR>1 {sum+=$6; count++} END {print "Average Throughput:", sum/count, "Mbps"}' metrics_test_*.csv
```

## Tùy Chỉnh Nâng Cao

### Thay đổi chunk size (trong code)
File: `src/core/Client/client_protocol.cpp`
```cpp
const size_t BUFFER_SIZE = 64*1024;  // 64KB (mặc định)
const size_t BUFFER_SIZE = 8*1024;   // 8KB (nhỏ hơn)
const size_t BUFFER_SIZE = 128*1024; // 128KB (lớn hơn)
```

### Filter file types trong test
Thêm vào script:
```bash
# Chỉ test file .jpg
files=($(find "$TEST_FOLDER" -type f -name "*.jpg"))

# Chỉ test file > 1MB
files=($(find "$TEST_FOLDER" -type f -size +1M))
```

### Export summary report
Thêm vào cuối script:
```bash
echo "=== SUMMARY REPORT ===" > summary.txt
awk -F',' 'NR>1 {
    sum_tp+=$6; sum_rtt+=$7; sum_lat+=$8; sum_loss+=$9; count++
} END {
    print "Average Throughput:", sum_tp/count, "Mbps"
    print "Average RTT:", sum_rtt/count, "ms"
    print "Average Latency:", sum_lat/count, "ms"
    print "Average Packet Loss:", sum_loss/count, "%"
}' "$OUTPUT_CSV" >> summary.txt
```

## Liên Hệ / Báo Lỗi
- Kiểm tra logs trong terminal
- Review file CSV output
- Check server logs: `./build/simp_server`
