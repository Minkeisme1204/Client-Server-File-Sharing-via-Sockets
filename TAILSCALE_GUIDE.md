# 🌐 Hướng Dẫn Sử Dụng Tailscale với File Transfer System

## Tổng Quan

Ứng dụng của bạn đã được tích hợp **tự động phát hiện Tailscale IP**, giúp kết nối client-server qua mạng riêng ảo Tailscale một cách dễ dàng và an toàn.

## 🎯 Tính Năng Mới

### 1. **Server GUI - Hiển thị Tailscale IP**
Server tự động hiển thị:
- **Tailscale IP**: Địa chỉ trên mạng Tailscale (100.x.x.x)
- **Local IP**: Địa chỉ trên mạng LAN

```
Server IP: Tailscale: 100.64.1.10 | Local: 192.168.1.50
```

### 2. **Client GUI - Kết Nối Nhanh với Tailscale**

#### Nút "🌐 Quick Connect (Tailscale)"
- Tự động phát hiện Tailscale IP
- Kết nối chỉ cần nhập port
- Hiển thị hướng dẫn cài đặt nếu chưa có Tailscale

#### Dialog Kết Nối Thông Minh
- Auto-fill Tailscale IP làm giá trị mặc định
- Hiển thị thông báo khi phát hiện Tailscale

## 📦 Cài Đặt Tailscale

### Linux (Ubuntu/Debian)
```bash
# Cài đặt Tailscale
curl -fsSL https://tailscale.com/install.sh | sh

# Đăng nhập và kết nối
sudo tailscale up

# Kiểm tra IP
tailscale ip -4
```

### Kiểm tra Tailscale đang chạy
```bash
tailscale status
```

Output mẫu:
```
100.64.1.10    myserver     ubuntu@     linux   -
100.64.1.20    myclient     ubuntu@     linux   -
```

## 🚀 Sử Dụng

### Bước 1: Khởi động Server
```bash
cd build
./server_gui
```

Server sẽ hiển thị:
- **Tailscale IP**: Dùng cho client kết nối từ xa (qua Tailscale)
- **Local IP**: Dùng cho client trong mạng LAN

### Bước 2: Kết Nối Client

#### Cách 1: Quick Connect (Khuyến nghị)
1. Mở Client GUI: `./client_gui`
2. Click nút **"🌐 Quick Connect (Tailscale)"**
3. Nhập port (mặc định: 9000)
4. Kết nối thành công!

#### Cách 2: Manual Connect
1. Click **"Connect"** button
2. Dialog tự động hiển thị: `100.64.1.10:9000` (Tailscale IP)
3. Nhấn OK để kết nối

#### Cách 3: Command Line
```bash
# Trong command box
connect 100.64.1.10 9000
```

## 🔍 Cách Hoạt Động

### Phát Hiện Tailscale IP (3 phương pháp)

**Phương pháp 1: Tailscale CLI**
```bash
tailscale ip -4
```

**Phương pháp 2: Network Interface**
Tìm interface `tailscale0` trong hệ thống

**Phương pháp 3: IP Range Detection**
Tìm địa chỉ trong dải `100.x.x.x` (Tailscale CGNAT range)

### Server Configuration
```cpp
// Server tự động lắng nghe trên TẤT CẢ interfaces
serverAddr.sin_addr.s_addr = INADDR_ANY;
```
Điều này có nghĩa:
- ✅ Chấp nhận kết nối từ Tailscale interface
- ✅ Chấp nhận kết nối từ Local network
- ✅ Chấp nhận kết nối từ localhost

## 💡 Use Cases

### 1. Kết Nối từ Xa An Toàn
```
Client (Nhà)    →  Tailscale  →  Server (Công ty)
100.64.1.20           VPN          100.64.1.10
```

**Ưu điểm:**
- 🔒 Mã hóa end-to-end
- 🚫 Không cần mở port firewall
- 🌐 Không cần IP công khai
- ⚡ Kết nối peer-to-peer trực tiếp

### 2. Chia Sẻ File Giữa Nhiều Máy
```
Laptop          Desktop         Server
100.64.1.20  ←→  100.64.1.30  ←→  100.64.1.10
```

### 3. Phát Triển và Test
```
Dev Machine  →  Tailscale  →  Staging Server
```

## 🛠️ Troubleshooting

### Không Tìm Thấy Tailscale IP

**Kiểm tra Tailscale đang chạy:**
```bash
sudo systemctl status tailscaled
```

**Khởi động lại Tailscale:**
```bash
sudo systemctl restart tailscaled
sudo tailscale up
```

**Kiểm tra IP:**
```bash
tailscale ip -4
ip addr show tailscale0
```

### Client Không Kết Nối Được

1. **Kiểm tra Server đang chạy**
```bash
# Trên server
./server_gui
# Xem log: "Server started on port 9000"
```

2. **Kiểm tra cả 2 máy cùng Tailscale network**
```bash
tailscale status
# Phải thấy cả client và server trong list
```

3. **Ping test**
```bash
ping 100.64.1.10
```

### Firewall Issues

Tailscale tự động bypass firewall, nhưng nếu vẫn gặp vấn đề:

```bash
# Cho phép traffic trên Tailscale interface
sudo ufw allow in on tailscale0
```

## 🎨 Code Implementation

### NetworkUtils Class
File: `app/common/NetworkUtils.h`

```cpp
class NetworkUtils {
public:
    static QString getTailscaleIP();      // Lấy Tailscale IP
    static QString getLocalIP();          // Lấy Local IP
    static bool isTailscaleAvailable();   // Check Tailscale installed
    static QStringList getAllNetworkIPs(); // All IPs
};
```

### Server Integration
File: `app/server/ServerWindow.cpp`

```cpp
QString ServerWindow::getServerIP() {
    QString tailscaleIP = NetworkUtils::getTailscaleIP();
    QString localIP = NetworkUtils::getLocalIP();
    
    if (!tailscaleIP.isEmpty()) {
        return QString("Tailscale: %1 | Local: %2")
            .arg(tailscaleIP).arg(localIP);
    }
    return QString("Local: %1").arg(localIP);
}
```

### Client Integration
File: `app/client/ClientWindow.cpp`

```cpp
void ClientWindow::onConnectClicked() {
    QString tailscaleIP = NetworkUtils::getTailscaleIP();
    QString defaultValue = QString("%1:9000").arg(
        tailscaleIP.isEmpty() ? NetworkUtils::getLocalIP() : tailscaleIP
    );
    // ... dialog với default value
}
```

## 📊 So Sánh: Tailscale vs Traditional VPN

| Feature | Tailscale | Traditional VPN |
|---------|-----------|-----------------|
| Setup | 1 phút | 30+ phút |
| Port Forwarding | Không cần | Phải config |
| Performance | Peer-to-peer | Qua server trung gian |
| Security | WireGuard | Varies |
| NAT Traversal | Tự động | Manual |
| Cost | Free (20 devices) | Paid |

## 🔐 Bảo Mật

### Tailscale Security Features
- **WireGuard Protocol**: State-of-the-art encryption
- **Key Exchange**: Automatic key rotation
- **ACLs**: Kiểm soát truy cập chi tiết
- **MFA**: Multi-factor authentication
- **Audit Logs**: Theo dõi mọi kết nối

### Best Practices
```bash
# 1. Giới hạn access với ACLs
# Tạo file acl.json trên Tailscale admin console

# 2. Enable MFA
tailscale up --accept-routes --ssh

# 3. Regular updates
sudo apt update && sudo apt upgrade tailscale
```

## 📚 Tài Nguyên Bổ Sung

- [Tailscale Documentation](https://tailscale.com/kb/)
- [Tailscale GitHub](https://github.com/tailscale/tailscale)
- [WireGuard Protocol](https://www.wireguard.com/)

## ✅ Checklist Triển Khai

Server Setup:
- [ ] Cài đặt Tailscale: `curl -fsSL https://tailscale.com/install.sh | sh`
- [ ] Đăng nhập: `sudo tailscale up`
- [ ] Lấy IP: `tailscale ip -4`
- [ ] Chạy server: `./server_gui`
- [ ] Xác nhận IP hiển thị đúng

Client Setup:
- [ ] Cài đặt Tailscale
- [ ] Đăng nhập cùng account
- [ ] Chạy client: `./client_gui`
- [ ] Sử dụng Quick Connect
- [ ] Test upload/download file

## 🎉 Kết Luận

Với tính năng tự động phát hiện Tailscale IP, ứng dụng của bạn giờ đây có thể:
- ✅ Kết nối an toàn qua Internet
- ✅ Không cần cấu hình firewall phức tạp
- ✅ Tự động phát hiện và sử dụng Tailscale
- ✅ Fallback về Local IP nếu không có Tailscale
- ✅ User-friendly với Quick Connect button

**Happy file sharing! 🚀**
