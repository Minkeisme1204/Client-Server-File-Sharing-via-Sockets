# Quy Trình Hoạt Động Socket trong Kết Nối Client-Server

## Tổng Quan

Tài liệu này mô tả chi tiết quy trình hoạt động của socket TCP trong kiến trúc client-server, từ khởi tạo đến đóng kết nối, bao gồm cả luồng dữ liệu và xử lý lỗi.

---

## 1. KHỞI TẠO VÀ THIẾT LẬP KẾT NỐI

### 1.1. Server Side - Khởi động và Lắng nghe

#### Bước 1: Tạo Socket
```cpp
int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
```
**Chức năng:**
- Tạo endpoint cho communication
- `AF_INET`: IPv4
- `SOCK_STREAM`: TCP (connection-oriented, reliable)
- Return: file descriptor (số nguyên dương) hoặc -1 nếu lỗi

**Điều xảy ra:**
- Kernel cấp phát cấu trúc dữ liệu cho socket
- Chưa có địa chỉ IP/port được gán

#### Bước 2: Set Socket Options
```cpp
int opt = 1;
setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```
**Chức năng:**
- `SO_REUSEADDR`: Cho phép bind lại port ngay sau khi server restart
- Tránh lỗi "Address already in use"

**Tại sao cần:**
- Khi server tắt, port có thể ở trạng thái TIME_WAIT
- Option này cho phép sử dụng lại port ngay lập tức

#### Bước 3: Bind Socket đến Address
```cpp
struct sockaddr_in serverAddr;
serverAddr.sin_family = AF_INET;
serverAddr.sin_addr.s_addr = INADDR_ANY;  // Lắng nghe trên tất cả interfaces
serverAddr.sin_port = htons(8080);        // Port 8080

bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
```
**Chức năng:**
- Gán địa chỉ IP và port cho socket
- `INADDR_ANY`: Chấp nhận kết nối từ mọi network interface
- `htons()`: Convert byte order từ host sang network (big-endian)

**Điều xảy ra:**
- Kernel ghi nhận socket này sẽ nhận packets đến port 8080
- Nếu port đã được sử dụng → lỗi

#### Bước 4: Listen - Chuyển sang Passive Mode
```cpp
listen(serverSocket, 10);  // backlog = 10
```
**Chức năng:**
- Đánh dấu socket ở chế độ passive (chờ kết nối)
- `backlog = 10`: Queue tối đa 10 kết nối pending

**Điều xảy ra:**
- Socket giờ có thể nhận connection requests
- Kernel tạo 2 queues:
  - **SYN queue**: Kết nối đang handshake (incomplete)
  - **Accept queue**: Kết nối đã hoàn thành handshake, chờ accept()

#### Bước 5: Accept - Chấp nhận Kết nối
```cpp
struct sockaddr_in clientAddr;
socklen_t clientAddrLen = sizeof(clientAddr);
int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
```
**Chức năng:**
- Lấy 1 kết nối từ accept queue
- **Blocking call**: Chờ đến khi có client kết nối
- Return: socket descriptor mới cho client

**Điều xảy ra:**
- Server lấy kết nối đã hoàn thành từ queue
- Tạo socket descriptor mới (`clientSocket`) dành riêng cho client này
- `serverSocket` vẫn tiếp tục lắng nghe kết nối mới
- Có thể lấy thông tin client: IP address, port

**Quan trọng:**
- Mỗi client có một socket riêng biệt
- Server socket ban đầu chỉ dùng để accept, không dùng để truyền data

---

### 1.2. Client Side - Kết nối đến Server

#### Bước 1: Tạo Socket
```cpp
int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
```
**Giống server**, tạo endpoint communication.

#### Bước 2: Cấu hình Server Address
```cpp
struct sockaddr_in serverAddr;
serverAddr.sin_family = AF_INET;
serverAddr.sin_port = htons(8080);
inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);  // Convert IP string
```
**Chức năng:**
- Xác định địa chỉ server muốn kết nối
- `inet_pton()`: Convert IP từ string sang binary

#### Bước 3: Connect - Thiết lập Kết nối
```cpp
connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
```
**Chức năng:**
- Khởi tạo TCP 3-way handshake
- **Blocking call**: Chờ đến khi kết nối được thiết lập hoặc timeout

**Điều xảy ra (TCP 3-Way Handshake):**

1. **Client → Server: SYN**
   - Client gửi SYN packet (synchronize)
   - Chứa sequence number ngẫu nhiên của client
   
2. **Server → Client: SYN-ACK**
   - Server acknowledge SYN của client
   - Gửi SYN của mình với sequence number riêng
   - Server chuyển kết nối vào accept queue
   
3. **Client → Server: ACK**
   - Client acknowledge SYN của server
   - Kết nối được thiết lập (ESTABLISHED)
   - `connect()` return thành công

**Sau khi thành công:**
- Socket ở trạng thái ESTABLISHED
- Cả 2 bên có thể send/receive data
- OS tự động gán port ngẫu nhiên cho client (ephemeral port)

---

## 2. TRUYỀN DỮ LIỆU

### 2.1. Gửi Dữ Liệu (Send)

#### Client hoặc Server gửi data:
```cpp
const char* message = "Hello Server";
ssize_t sent = send(socket, message, strlen(message), 0);
```

**Chức năng:**
- Gửi data qua socket
- Return: số bytes đã gửi thực tế

**Điều xảy ra:**

1. **Application Layer:**
   - Data được copy từ user space vào kernel buffer

2. **Transport Layer (TCP):**
   - TCP chia data thành segments
   - Thêm TCP header (source port, dest port, sequence number, checksum)
   - Đặt vào send buffer
   
3. **Network Layer (IP):**
   - Thêm IP header (source IP, dest IP)
   - Routing: Xác định interface để gửi

4. **Link Layer:**
   - Thêm Ethernet header (MAC addresses)
   - Gửi frames qua network

5. **Acknowledgment:**
   - Receiver gửi ACK packet xác nhận đã nhận
   - Nếu không nhận ACK → TCP tự động retransmit

**Lưu ý:**
- `send()` có thể **không gửi hết** data nếu buffer đầy
- Cần kiểm tra return value và gửi tiếp phần còn lại
- Flag `MSG_NOSIGNAL`: Tránh SIGPIPE khi peer đóng kết nối

**Ví dụ gửi đầy đủ:**
```cpp
size_t totalSent = 0;
while (totalSent < dataSize) {
    ssize_t sent = send(socket, data + totalSent, dataSize - totalSent, MSG_NOSIGNAL);
    if (sent < 0) {
        if (errno == EINTR) continue;  // Interrupted, retry
        // Handle error
        break;
    }
    totalSent += sent;
}
```

---

### 2.2. Nhận Dữ Liệu (Receive)

#### Client hoặc Server nhận data:
```cpp
char buffer[1024];
ssize_t received = recv(socket, buffer, sizeof(buffer), 0);
```

**Chức năng:**
- Đọc data từ socket receive buffer
- **Blocking call**: Chờ đến khi có data
- Return: 
  - `> 0`: Số bytes nhận được
  - `0`: Peer đã đóng kết nối (EOF)
  - `-1`: Lỗi

**Điều xảy ra:**

1. **Packets đến:**
   - Network card nhận packets
   - Kernel kiểm tra checksum
   - Data được đặt vào receive buffer của socket

2. **TCP Processing:**
   - Reorder packets theo sequence number
   - Loại bỏ duplicates
   - Gửi ACK về sender
   - Thực hiện flow control (window size)

3. **Application đọc:**
   - `recv()` copy data từ kernel buffer sang user buffer
   - Giải phóng space trong kernel buffer
   - Update TCP window để sender biết có thể gửi thêm

**Lưu ý:**
- `recv()` có thể nhận **ít hơn** size yêu cầu
- TCP là stream protocol, không có message boundaries
- Cần đọc đủ data mong muốn

**Ví dụ nhận đầy đủ:**
```cpp
size_t totalReceived = 0;
while (totalReceived < expectedSize) {
    ssize_t received = recv(socket, buffer + totalReceived, 
                           expectedSize - totalReceived, 0);
    if (received < 0) {
        if (errno == EINTR) continue;  // Retry
        // Handle error
        break;
    }
    if (received == 0) {
        // Connection closed
        break;
    }
    totalReceived += received;
}
```

---

## 3. PROTOCOL APPLICATION LAYER

Trong hệ thống file sharing, có protocol tùy chỉnh trên TCP:

### 3.1. Command Protocol

#### LIST Command Flow:

**Client:**
```
1. Send CMD_LIST (1 byte)
   ↓
2. Wait for response
   ↓
3. Receive file count (4 bytes, uint32_t)
   ↓
4. For each file:
   - Receive filename (256 bytes, null-terminated)
   ↓
5. Display files to user
```

**Server:**
```
1. Receive CMD_LIST (1 byte)
   ↓
2. Scan shared directory
   ↓
3. Send file count (4 bytes)
   ↓
4. For each file:
   - Send filename (256 bytes buffer)
   ↓
5. Continue waiting for next command
```

#### GET Command Flow:

**Client:**
```
1. Send CMD_GET (1 byte)
   ↓
2. Send filename (256 bytes)
   ↓
3. Receive file size (8 bytes, uint64_t)
   ↓
4. If fileSize == 0:
   - File not found, abort
   ↓
5. Receive file data in chunks
   ↓
6. Write to local file
   ↓
7. Verify total bytes received
```

**Server:**
```
1. Receive CMD_GET (1 byte)
   ↓
2. Receive filename (256 bytes)
   ↓
3. Check if file exists
   ↓
4. Send file size (8 bytes)
   - If not found, send 0
   ↓
5. If file exists:
   - Open file
   - Read chunks (8KB buffer)
   - Send each chunk
   - Close file
   ↓
6. Continue waiting for next command
```

#### PUT Command Flow:

**Client:**
```
1. Send CMD_PUT (1 byte)
   ↓
2. Send filename (256 bytes)
   ↓
3. Send file size (8 bytes)
   ↓
4. Read file in chunks
   ↓
5. Send each chunk
   ↓
6. Wait for completion
```

**Server:**
```
1. Receive CMD_PUT (1 byte)
   ↓
2. Receive filename (256 bytes)
   ↓
3. Receive file size (8 bytes)
   ↓
4. Create/open file for writing
   ↓
5. Receive data in chunks
   ↓
6. Write each chunk to file
   ↓
7. Close file when done
   ↓
8. Continue waiting for next command
```

---

## 4. ĐÓNG KẾT NỐI

### 4.1. Graceful Shutdown (4-Way Handshake)

#### Client muốn đóng kết nối:

**Bước 1: Client → Server (FIN)**
```cpp
close(clientSocket);  // or shutdown(clientSocket, SHUT_WR)
```
- Client gửi FIN packet
- Báo hiệu: "Tôi không gửi data nữa"
- Client vẫn có thể nhận data từ server

**Bước 2: Server → Client (ACK)**
- Server acknowledge FIN
- Server vẫn có thể gửi data

**Bước 3: Server → Client (FIN)**
```cpp
close(serverSocket);
```
- Server gửi FIN khi đã xong việc
- Báo hiệu: "Tôi cũng không gửi data nữa"

**Bước 4: Client → Server (ACK)**
- Client acknowledge FIN của server
- Kết nối hoàn toàn đóng

**Trạng thái TIME_WAIT:**
- Client ở trạng thái TIME_WAIT (2-4 phút)
- Đảm bảo ACK cuối đến server
- Tránh packets cũ ảnh hưởng kết nối mới

---

### 4.2. Abrupt Close (Reset)

#### Connection Reset:
```cpp
// Set SO_LINGER with timeout = 0
struct linger linger_opt = {1, 0};
setsockopt(socket, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
close(socket);
```
- Gửi RST packet thay vì FIN
- Đóng ngay lập tức, không graceful
- Data trong send buffer bị mất

**Khi nào xảy ra:**
- Application crash
- `kill -9` process
- Connection timeout
- Nhận data trên closed socket

---

## 5. XỬ LÝ LỖI VÀ EDGE CASES

### 5.1. Connection Timeout

**Client connect timeout:**
```cpp
// Set timeout before connect
struct timeval timeout;
timeout.tv_sec = 5;
timeout.tv_usec = 0;
setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
```

**TCP retransmission:**
- TCP tự động retransmit nếu không nhận ACK
- Exponential backoff: 1s, 2s, 4s, 8s...
- Sau nhiều lần → connection timeout

---

### 5.2. Broken Pipe (SIGPIPE)

**Vấn đề:**
```cpp
send(socket, data, size, 0);  // Socket đã bị đóng bởi peer
```
- OS gửi SIGPIPE signal
- Default: Terminate process

**Giải pháp:**
```cpp
// Option 1: Ignore signal
signal(SIGPIPE, SIG_IGN);

// Option 2: Use MSG_NOSIGNAL flag
send(socket, data, size, MSG_NOSIGNAL);
// Return -1 với errno = EPIPE thay vì signal
```

---

### 5.3. Partial Send/Receive

**Vấn đề:** Send/Recv không đảm bảo xử lý hết data

**Best Practice:**
```cpp
// Wrapper function
ssize_t sendAll(int socket, const uint8_t* data, size_t size) {
    size_t totalSent = 0;
    while (totalSent < size) {
        ssize_t sent = send(socket, data + totalSent, 
                           size - totalSent, MSG_NOSIGNAL);
        if (sent < 0) {
            if (errno == EINTR) continue;  // Interrupted system call
            if (errno == EPIPE || errno == ECONNRESET) {
                return -1;  // Connection closed
            }
            return -1;  // Other error
        }
        totalSent += sent;
    }
    return totalSent;
}

ssize_t recvAll(int socket, uint8_t* buffer, size_t size) {
    size_t totalReceived = 0;
    while (totalReceived < size) {
        ssize_t received = recv(socket, buffer + totalReceived, 
                               size - totalReceived, 0);
        if (received < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (received == 0) {
            // Connection closed by peer
            return (totalReceived == 0) ? 0 : -1;
        }
        totalReceived += received;
    }
    return totalReceived;
}
```

---

### 5.4. Detect Connection Close

**Các cách phát hiện:**

1. **recv() returns 0:**
   - Peer đã gọi close()
   - Clean shutdown

2. **recv() returns -1:**
   - `errno == ECONNRESET`: Connection reset by peer
   - `errno == ETIMEDOUT`: Connection timeout
   - `errno == EPIPE`: Broken pipe

3. **send() fails:**
   - Peer không còn nhận data
   - Thường là connection đã đóng

---

## 6. MULTI-THREADED SERVER ARCHITECTURE

### 6.1. Main Server Thread

```cpp
while (serverRunning) {
    // Accept blocking
    int clientFd = accept(serverSocket, ...);
    
    if (clientFd < 0) continue;
    
    // Create new session for client
    ClientSession* session = new ClientSession(clientFd, clientAddr, sharedDir);
    session->start();  // Spawn new thread
    
    // Store session for management
    sessions.push_back(session);
}
```

**Chức năng:**
- Accept kết nối mới
- Tạo ClientSession
- Tiếp tục lắng nghe

---

### 6.2. Client Session Thread

```cpp
void ClientSession::handleSession() {
    while (active) {
        // Read command
        uint8_t cmd;
        if (recv(clientFd, &cmd, 1, 0) <= 0) {
            break;  // Client disconnected
        }
        
        // Process command
        switch (cmd) {
            case CMD_LIST:
                handleList();
                break;
            case CMD_GET:
                handleGet();
                break;
            case CMD_PUT:
                handlePut();
                break;
        }
    }
    
    // Cleanup
    close(clientFd);
}
```

**Đặc điểm:**
- Mỗi client chạy trong thread riêng
- Không block server chính
- Xử lý nhiều clients đồng thời

---

### 6.3. Thread Safety

**Vấn đề:**
- Nhiều threads truy cập shared resources
- Race conditions
- Deadlocks

**Giải pháp:**

1. **Mutex cho shared data:**
```cpp
std::mutex sessionsMutex;

void addSession(ClientSession* session) {
    std::lock_guard<std::mutex> lock(sessionsMutex);
    sessions.push_back(session);
}
```

2. **Thread-safe containers:**
```cpp
std::vector<std::unique_ptr<ClientSession>> sessions;
```

3. **Atomic operations:**
```cpp
std::atomic<bool> serverRunning{true};
```

---

## 7. BUFFER MANAGEMENT

### 7.1. Kernel Buffers

**Send Buffer (SO_SNDBUF):**
- Default: ~16KB - 64KB
- Chứa data chưa được ACK
- Khi đầy → send() blocks

**Receive Buffer (SO_RCVBUF):**
- Default: ~16KB - 64KB
- Chứa data đã nhận nhưng chưa được read
- TCP window size phụ thuộc buffer này

**Tùy chỉnh:**
```cpp
int bufferSize = 256 * 1024;  // 256KB
setsockopt(socket, SOL_SOCKET, SO_SNDBUF, &bufferSize, sizeof(bufferSize));
setsockopt(socket, SOL_SOCKET, SO_RCVBUF, &bufferSize, sizeof(bufferSize));
```

---

### 7.2. Application Buffers

**Chunk-based Transfer:**
```cpp
const size_t CHUNK_SIZE = 8192;  // 8KB
uint8_t buffer[CHUNK_SIZE];

while (totalSent < fileSize) {
    size_t toRead = std::min(CHUNK_SIZE, fileSize - totalSent);
    ssize_t bytesRead = fread(buffer, 1, toRead, file);
    sendAll(socket, buffer, bytesRead);
    totalSent += bytesRead;
}
```

**Tại sao dùng chunks:**
- Tiết kiệm memory (không load toàn bộ file)
- Progress tracking
- Có thể pause/resume

---

## 8. FLOW CONTROL VÀ CONGESTION CONTROL

### 8.1. TCP Flow Control (Sliding Window)

**Receiver advertises window size:**
```
Window = Buffer Size - Data In Buffer
```

**Sender:**
- Không gửi quá window size
- Đợi ACK để giải phóng window

**Zero Window:**
- Receiver buffer đầy
- Sender dừng gửi
- Chờ window update

---

### 8.2. Congestion Control

**Slow Start:**
- Bắt đầu với cwnd (congestion window) nhỏ
- Tăng exponential sau mỗi ACK
- Đến threshold → chuyển sang Congestion Avoidance

**Congestion Avoidance:**
- Tăng cwnd linear
- Tránh làm nghẽn network

**Fast Retransmit & Recovery:**
- Nhận 3 duplicate ACKs
- Retransmit ngay không chờ timeout

---

## 9. KEEPALIVE MECHANISM

### 9.1. TCP Keepalive

**Vấn đề:**
- Peer crash không gửi FIN
- Connection appears ESTABLISHED
- Không biết khi nào đóng

**Giải pháp:**
```cpp
int keepalive = 1;
int keepidle = 60;      // 60s trước keepalive đầu tiên
int keepinterval = 10;  // 10s giữa các probes
int keepcount = 3;      // 3 probes trước khi timeout

setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));
setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle));
setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, &keepinterval, sizeof(keepinterval));
setsockopt(socket, IPPROTO_TCP, TCP_KEEPCNT, &keepcount, sizeof(keepcount));
```

**Hoạt động:**
1. Không có data trong 60s → gửi keepalive probe
2. Không nhận response → gửi lại sau 10s
3. Sau 3 lần không response → đóng connection

---

### 9.2. Application-Level Heartbeat

**Alternative approach:**
```cpp
// Server sends PING every 30s
while (running) {
    send(clientFd, "PING", 4, 0);
    sleep(30);
}

// Client must respond with PONG
```

**Ưu điểm:**
- Kiểm soát timeout chính xác
- Detect application-level issues
- Protocol-aware

---

## 10. DEBUGGING VÀ MONITORING

### 10.1. Socket States

```bash
# Xem tất cả connections
netstat -ant

# Hoặc
ss -ant
```

**Các states:**
- `LISTEN`: Server đang lắng nghe
- `SYN_SENT`: Client đang connect
- `SYN_RECV`: Server đã nhận SYN
- `ESTABLISHED`: Kết nối đang hoạt động
- `FIN_WAIT_1`, `FIN_WAIT_2`: Đang đóng
- `TIME_WAIT`: Đã đóng, chờ cleanup
- `CLOSE_WAIT`: Peer đã đóng, đang chờ local close

---

### 10.2. tcpdump - Packet Capture

```bash
# Capture packets trên port 8080
sudo tcpdump -i lo port 8080 -A

# Chi tiết hơn
sudo tcpdump -i lo port 8080 -X -vv
```

**Xem được:**
- SYN, SYN-ACK, ACK handshake
- Data packets
- FIN handshake
- Retransmissions

---

### 10.3. strace - System Call Tracing

```bash
# Trace socket system calls
strace -e trace=socket,bind,listen,accept,connect,send,recv ./server_test
```

**Xem được:**
- Thứ tự system calls
- Return values
- Errors (errno)

---

## 11. PERFORMANCE OPTIMIZATION

### 11.1. Nagle's Algorithm

**Vấn đề:**
- Gửi nhiều small packets kém hiệu quả
- Overhead: header (40 bytes) > data (1 byte)

**Nagle's Algorithm:**
- Buffer small writes
- Gửi khi đủ MSS hoặc nhận ACK

**Disable (nếu cần low latency):**
```cpp
int flag = 1;
setsockopt(socket, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
```

---

### 11.2. Cork Mode

**Buffer multiple writes:**
```cpp
// Enable
int flag = 1;
setsockopt(socket, IPPROTO_TCP, TCP_CORK, &flag, sizeof(flag));

send(socket, header, headerSize, 0);
send(socket, data, dataSize, 0);
send(socket, footer, footerSize, 0);

// Disable - flush all data
flag = 0;
setsockopt(socket, IPPROTO_TCP, TCP_CORK, &flag, sizeof(flag));
```

**Kết quả:** 3 sends → 1 TCP packet

---

### 11.3. Zero-Copy (sendfile)

**Traditional approach:**
```cpp
read(filefd, buffer, size);    // Copy to user space
send(socket, buffer, size, 0);  // Copy to kernel space
```
→ 2 copies, context switches

**Zero-copy:**
```cpp
#include <sys/sendfile.h>
off_t offset = 0;
sendfile(socket, filefd, &offset, fileSize);
```
→ Direct kernel → kernel transfer, không qua user space

**Hiệu suất:** Nhanh hơn 2-3 lần cho file transfer lớn

---

## 12. ERROR HANDLING BEST PRACTICES

### 12.1. Comprehensive Error Checking

```cpp
ssize_t bytes = recv(socket, buffer, size, 0);

if (bytes < 0) {
    switch (errno) {
        case EINTR:
            // Signal interrupted, retry
            continue;
            
        case EAGAIN:  // or EWOULDBLOCK
            // Non-blocking socket, no data available
            // Wait and retry
            break;
            
        case ECONNRESET:
            // Connection reset by peer
            std::cerr << "Connection reset\n";
            return -1;
            
        case ETIMEDOUT:
            // Connection timeout
            std::cerr << "Connection timeout\n";
            return -1;
            
        default:
            std::cerr << "Recv error: " << strerror(errno) << "\n";
            return -1;
    }
} else if (bytes == 0) {
    // Clean disconnect
    std::cout << "Peer closed connection\n";
    return 0;
}
```

---

### 12.2. Resource Cleanup (RAII)

```cpp
class SocketGuard {
    int fd_;
public:
    SocketGuard(int fd) : fd_(fd) {}
    ~SocketGuard() {
        if (fd_ >= 0) {
            close(fd_);
        }
    }
    int get() const { return fd_; }
};

// Usage
SocketGuard socket(::socket(AF_INET, SOCK_STREAM, 0));
if (socket.get() < 0) return;
// Automatically closed when out of scope
```

---

## TÓM TẮT WORKFLOW

### Server Initialization
```
socket() → setsockopt() → bind() → listen() → [loop: accept() → spawn thread]
```

### Client Connection
```
socket() → connect() → [data exchange] → close()
```

### Data Exchange
```
Client: send(CMD) → send(data) → recv(response)
Server: recv(CMD) → process → send(response)
```

### Connection Termination
```
FIN → ACK → FIN → ACK → TIME_WAIT → CLOSED
```

---

## KẾT LUẬN

Socket programming là nền tảng của network communication. Hiểu rõ workflow từ low-level (TCP handshake, packet flow) đến high-level (application protocol) giúp:

1. **Debug hiệu quả**: Biết chính xác điểm nào có thể fail
2. **Optimize performance**: Buffer tuning, zero-copy, keepalive
3. **Handle errors gracefully**: Timeout, disconnect, retransmission
4. **Design robust protocols**: Message boundaries, flow control

Hệ thống file sharing này demonstrate đầy đủ các khái niệm trên trong một ứng dụng thực tế.
