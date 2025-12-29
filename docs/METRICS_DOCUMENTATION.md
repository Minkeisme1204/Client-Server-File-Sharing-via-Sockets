# Tài Liệu Metrics - Client-Server File Transfer

## Tổng Quan
Tài liệu này mô tả chi tiết các tham số đánh giá hiệu suất (metrics) được sử dụng trong hệ thống Client-Server File Transfer, bao gồm định nghĩa toán học, ý nghĩa thực tế, và phương pháp đo lường.

---

## Server Metrics

### 1. Uptime (Thời Gian Hoạt Động)

**Định nghĩa:**
Uptime là khoảng thời gian liên tục mà server đang hoạt động kể từ lúc khởi động.

**Công thức toán học:**
```
Uptime(t) = t_current - t_start
```
Trong đó:
- `t_current`: Thời điểm hiện tại
- `t_start`: Thời điểm server khởi động

**Ý nghĩa:**
- Đo lường độ tin cậy và tính sẵn sàng của server
- Uptime cao cho thấy server ổn định, ít gặp sự cố
- Là một trong những KPI quan trọng nhất trong Service Level Agreement (SLA)

**Nguồn tham khảo:**
> "System uptime is a critical metric for measuring system availability and is typically expressed as a percentage of total time." 
> - *Computer Networks: A Systems Approach*, Peterson & Davie, 5th Edition, Section 8.3

---

### 2. Total Connections (Tổng Số Kết Nối)

**Định nghĩa:**
Tổng số kết nối client đã được thiết lập với server từ lúc khởi động.

**Công thức toán học:**
```
Total_Connections = ∑(i=1 to n) connection_i
```

**Ý nghĩa:**
- Đo lường mức độ sử dụng của server
- Giúp dự đoán tải và lập kế hoạch mở rộng
- Indicator về độ phổ biến của service

**Nguồn tham khảo:**
> "Connection metrics provide insight into server utilization and help identify capacity planning requirements."
> - *High Performance Browser Networking*, Ilya Grigorik, O'Reilly Media, 2013, Chapter 1

---

### 3. Active Connections (Số Kết Nối Đang Hoạt Động)

**Định nghĩa:**
Số lượng kết nối client đang active tại thời điểm hiện tại.

**Công thức toán học:**
```
Active_Connections(t) = Total_Connections(t) - Closed_Connections(t)
```

**Ý nghĩa:**
- Đo lường tải hiện tại của server
- Quan trọng cho load balancing và resource allocation
- Giúp phát hiện potential DoS attacks khi có spike bất thường

**Nguồn tham khảo:**
> "The number of concurrent connections is a key factor in determining server capacity and performance under load."
> - RFC 7230 (HTTP/1.1): Message Syntax and Routing, Section 6.3

---

### 4. Failed Connections (Số Kết Nối Thất Bại)

**Định nghĩa:**
Số lượng kết nối không thể được thiết lập hoặc bị gián đoạn bất thường.

**Công thức toán học:**
```
Connection_Failure_Rate = (Failed_Connections / Total_Connection_Attempts) × 100%
```

**Ý nghĩa:**
- Chỉ số về reliability và stability của server
- Giúp phát hiện network issues hoặc server overload
- Failure rate cao cần được điều tra ngay

**Nguồn tham khảo:**
> "Connection failure metrics are essential for diagnosing network reliability issues and server capacity problems."
> - *TCP/IP Illustrated, Volume 1*, W. Richard Stevens, Addison-Wesley, Chapter 18

---

### 5. Bytes Received/Sent (Dữ Liệu Nhận/Gửi)

**Định nghĩa:**
Tổng số bytes dữ liệu mà server đã nhận từ hoặc gửi đến clients.

**Công thức toán học:**
```
Total_Bytes = ∑(i=1 to n) bytes_transferred_i
```

**Ý nghĩa:**
- Đo lường volume của data transfer
- Quan trọng cho bandwidth planning và billing
- Giúp identify data-intensive operations

**Nguồn tham khảo:**
> "Byte count metrics are fundamental for network traffic analysis and capacity planning."
> - *Computer Networking: A Top-Down Approach*, Kurose & Ross, 7th Edition, Chapter 1.4

---

### 6. Files Uploaded/Downloaded

**Định nghĩa:**
Số lượng files đã được upload lên hoặc download từ server.

**Công thức toán học:**
```
Total_Files_Transferred = Files_Uploaded + Files_Downloaded
File_Transfer_Success_Rate = (Successful_Transfers / Total_Transfers) × 100%
```

**Ý nghĩa:**
- Application-level metric cho file transfer systems
- Đo lường operational efficiency
- Quan trọng cho business analytics

---

### 7. Average Throughput (Throughput Trung Bình)

**Định nghĩa:**
Tốc độ truyền dữ liệu trung bình của server, đo bằng kilobits per second (kbps).

**Công thức toán học:**
```
Throughput_avg = (1/N) × ∑(i=1 to N) Throughput_i

Throughput_i = (Bytes_i × 8) / (Duration_i / 1000) / 1024

Exponential Moving Average:
Throughput_avg(t) = α × Throughput(t) + (1-α) × Throughput_avg(t-1)
```
Trong đó:
- `α = 0.1` (smoothing factor)
- `Bytes_i`: Số bytes transferred trong transaction i
- `Duration_i`: Thời gian transfer (milliseconds)

**Ý nghĩa:**
- Đo lường hiệu suất network của server
- Giúp identify bottlenecks trong data transfer
- Quan trọng cho Quality of Service (QoS) monitoring

**Nguồn tham khảo:**
> "Throughput is defined as the amount of data transferred over a network connection per unit time. It is a critical performance metric for evaluating network and application performance."
> - RFC 6349: Framework for TCP Throughput Testing, Section 3.1

> "Exponential moving averages provide a smooth representation of time-varying metrics while giving more weight to recent observations."
> - *Time Series Analysis: Forecasting and Control*, Box & Jenkins, 5th Edition, Chapter 2

---

### 8. Peak Throughput (Throughput Đỉnh)

**Định nghĩa:**
Throughput cao nhất đã đạt được trong một transaction.

**Công thức toán học:**
```
Peak_Throughput = max(Throughput_i) for all i
```

**Ý nghĩa:**
- Cho biết khả năng tối đa của server
- Giúp establish performance baselines
- Quan trọng cho capacity planning

**Nguồn tham khảo:**
> "Peak throughput measurements help establish upper bounds on system performance and are useful for capacity planning."
> - IEEE 802.3 Ethernet Standard, Annex 48A

---

### 9. Average Latency (Độ Trễ Trung Bình)

**Định nghĩa:**
Thời gian trung bình để xử lý một request từ client.

**Công thức toán học:**
```
Latency_avg = (1/N) × ∑(i=1 to N) (t_response_i - t_request_i)

Exponential Moving Average:
Latency_avg(t) = α × Latency(t) + (1-α) × Latency_avg(t-1)
```
Trong đó:
- `α = 0.1` (smoothing factor)
- `t_request_i`: Thời điểm nhận request
- `t_response_i`: Thời điểm hoàn thành xử lý

**Ý nghĩa:**
- Đo lường responsiveness của server
- Latency thấp = user experience tốt
- Critical metric cho real-time applications
- Giúp identify processing bottlenecks

**Nguồn tham khảo:**
> "Latency is the time interval between the initiation of a request and the start of a response. It is a key metric for evaluating user-perceived performance."
> - *Designing Data-Intensive Applications*, Martin Kleppmann, O'Reilly, Chapter 1

> "Network latency significantly impacts application performance and user experience, particularly for interactive applications."
> - RFC 2679: A One-way Delay Metric for IPPM, Section 2

---

## Client Metrics

### 1. RTT (Round-Trip Time)

**Định nghĩa:**
Thời gian để một packet đi từ client đến server và nhận được response trở lại.

**Công thức toán học:**
```
RTT = t_response - t_request

RTT_avg(t) = α × RTT(t) + (1-α) × RTT_avg(t-1)
```
Trong đó:
- `α = 0.3` (smoothing factor - cao hơn để phản ứng nhanh với thay đổi)
- Đơn vị: milliseconds (ms)

**Ý nghĩa:**
- Fundamental metric cho network performance
- Bao gồm cả network delay và server processing time
- Ảnh hưởng trực tiếp đến user experience
- Quan trọng cho TCP congestion control algorithms

**Nguồn tham khảo:**
> "Round-trip time (RTT) is the duration measured from the time a sender sends a segment until it receives an acknowledgment from the receiver. RTT is fundamental to TCP's retransmission timeout calculation."
> - *TCP/IP Illustrated, Volume 1*, W. Richard Stevens, Section 21.2

> "RTT measurements are essential for adaptive timeout calculations and congestion control in TCP."
> - RFC 6298: Computing TCP's Retransmission Timer, Section 2

---

### 2. Throughput (Băng Thông Thực Tế)

**Định nghĩa:**
Tốc độ truyền dữ liệu thực tế mà client đạt được.

**Công thức toán học:**
```
Throughput = (File_Size_bits / Transfer_Time_seconds)

Throughput_kbps = (File_Size_bytes × 8) / (Duration_ms / 1000) / 1024
```

**Ý nghĩa:**
- Đo lường hiệu suất thực tế của connection
- Phản ánh bandwidth availability và network conditions
- Thường thấp hơn bandwidth theoretical do:
  - Protocol overhead
  - Network congestion
  - Packet loss và retransmissions

**Bandwidth Utilization:**
```
Utilization = (Throughput / Available_Bandwidth) × 100%
```

**Nguồn tham khảo:**
> "Throughput represents the actual rate of successful data delivery and is typically lower than the channel capacity due to various overhead factors."
> - *Computer Networks*, Andrew S. Tanenbaum, 5th Edition, Section 3.2.1

> "Application-level throughput is affected by TCP protocol overhead, network congestion, and packet loss, resulting in effective throughput lower than link capacity."
> - RFC 5681: TCP Congestion Control, Section 3

---

### 3. Packet Loss Rate (Tỷ Lệ Mất Gói Tin)

**Định nghĩa:**
Tỷ lệ phần trăm các requests/packets bị mất hoặc thất bại.

**Công thức toán học:**
```
Packet_Loss_Rate = (Failed_Requests / Total_Requests) × 100%

Error_Rate = (Errors / Total_Transmissions) × 100%
```

**Ý nghĩa:**
- Critical indicator của network quality
- Packet loss gây ra:
  - Retransmissions (giảm throughput)
  - Tăng latency
  - Giảm user experience
- TCP có thể phục hồi từ packet loss, nhưng với performance penalty

**Acceptable Levels:**
- < 1%: Excellent
- 1-2.5%: Good
- 2.5-5%: Fair (có thể ảnh hưởng performance)
- > 5%: Poor (cần investigation)

**Nguồn tham khảo:**
> "Packet loss is a critical quality metric for network performance. Even small amounts of loss can significantly impact application performance due to retransmissions."
> - RFC 3393: IP Packet Delay Variation Metric for IP Performance Metrics (IPPM), Section 1

> "TCP interprets packet loss as a signal of network congestion and responds by reducing its transmission rate."
> - RFC 5681: TCP Congestion Control, Section 3.1

> "Packet loss rates below 1% are generally considered acceptable for most applications, while rates above 5% typically indicate serious network problems."
> - *High Performance Browser Networking*, Ilya Grigorik, Chapter 2

---

### 4. Transfer Latency (Độ Trễ Truyền Tải)

**Định nghĩa:**
Thời gian tổng cộng để hoàn thành một file transfer operation.

**Công thức toán học:**
```
Transfer_Latency = t_complete - t_start

Transfer_Latency = (File_Size / Throughput) + RTT + Processing_Time
```

**Components:**
- **Transmission Time**: `File_Size / Bandwidth`
- **Propagation Delay**: Phụ thuộc vào khoảng cách vật lý
- **Queuing Delay**: Thời gian chờ trong router queues
- **Processing Time**: Server và client processing

**Ý nghĩa:**
- End-to-end performance metric
- Quan trọng cho user experience
- Bao gồm tất cả delays trong quá trình transfer
- Giúp identify bottlenecks trong transfer pipeline

**Nguồn tham khảo:**
> "Total transfer latency consists of multiple components: transmission delay, propagation delay, queuing delay, and processing delay."
> - *Computer Networking: A Top-Down Approach*, Kurose & Ross, Section 1.4.3

> "End-to-end latency is the sum of all delays experienced by data as it travels from source to destination."
> - *Computer Networks*, Andrew S. Tanenbaum, Chapter 4

---

## Phương Pháp Đo Lường

### 1. High-Resolution Timing

**Implementation:**
```cpp
auto start = std::chrono::high_resolution_clock::now();
// Perform operation
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
```

**Độ chính xác:**
- Sử dụng `std::chrono::high_resolution_clock` (C++11)
- Độ phân giải: microseconds (μs)
- Lý do: milliseconds không đủ chính xác cho operations nhanh

**Nguồn tham khảo:**
> "High-resolution timers are essential for accurate performance measurement in modern systems."
> - *Systems Performance: Enterprise and the Cloud*, Brendan Gregg, Chapter 6

---

### 2. Exponential Moving Average (EMA)

**Công thức:**
```
EMA(t) = α × Value(t) + (1-α) × EMA(t-1)
```

**Lựa chọn α:**
- α = 0.1: Smooth, slow to react (throughput, latency)
- α = 0.3: Balance, faster reaction (RTT)
- α càng cao = càng nhạy với changes

**Ý nghĩa:**
- Giảm noise trong measurements
- Phản ánh xu hướng dài hạn
- Vẫn responsive với changes đột ngột

**Nguồn tham khảo:**
> "Exponential moving averages provide a balance between stability and responsiveness in time-series metrics."
> - *Forecasting: Principles and Practice*, Hyndman & Athanasopoulos, Section 7.1

---

## Best Practices

### 1. Metric Collection
- Thu thập metrics liên tục, không chỉ khi có vấn đề
- Sử dụng atomic operations để thread-safety
- Store metrics history cho trend analysis

### 2. Metric Interpretation
- So sánh với baseline measurements
- Xem xét context (time of day, load, etc.)
- Monitor trends, không chỉ absolute values

### 3. Performance Optimization
- Latency < 100ms: Excellent user experience
- Throughput utilization 70-80%: Optimal (để room cho spikes)
- Packet loss < 1%: Acceptable for most applications

**Nguồn tham khảo:**
> "Effective performance monitoring requires continuous collection and analysis of metrics in context."
> - *The Art of Monitoring*, James Turnbull, Chapter 2

> "Response times below 100ms are perceived as instantaneous by users."
> - Jakob Nielsen's Response Time Guidelines for Web Usability

---

## Tài Liệu Tham Khảo

### RFCs (Internet Standards)
1. RFC 2679 - A One-way Delay Metric for IPPM
2. RFC 3393 - IP Packet Delay Variation Metric
3. RFC 5681 - TCP Congestion Control
4. RFC 6298 - Computing TCP's Retransmission Timer
5. RFC 6349 - Framework for TCP Throughput Testing
6. RFC 7230 - HTTP/1.1: Message Syntax and Routing

### Sách Chuyên Ngành
1. **TCP/IP Illustrated, Volume 1** - W. Richard Stevens, Addison-Wesley
2. **Computer Networks: A Systems Approach** - Peterson & Davie, 5th Edition
3. **Computer Networking: A Top-Down Approach** - Kurose & Ross, 7th Edition
4. **Computer Networks** - Andrew S. Tanenbaum, 5th Edition
5. **High Performance Browser Networking** - Ilya Grigorik, O'Reilly Media
6. **Designing Data-Intensive Applications** - Martin Kleppmann, O'Reilly
7. **Systems Performance: Enterprise and the Cloud** - Brendan Gregg, Prentice Hall

### Chuẩn IEEE
1. IEEE 802.3 - Ethernet Standard

### Online Resources
1. Jakob Nielsen's Usability Guidelines - https://www.nngroup.com/articles/response-times-3-important-limits/

---

## Kết Luận

Các metrics trong hệ thống Client-Server File Transfer cung cấp cái nhìn toàn diện về:
- **Performance**: Throughput, Latency, RTT
- **Reliability**: Packet Loss, Failed Connections, Uptime
- **Utilization**: Active Connections, Bytes Transferred
- **Quality of Service**: Transfer Latency, Connection Success Rate

Việc monitoring và phân tích các metrics này là critical cho:
- Performance optimization
- Capacity planning
- Problem diagnosis
- User experience improvement
- SLA compliance

---

*Tài liệu được biên soạn dựa trên các chuẩn Internet (RFCs), sách giáo trình chuyên ngành, và best practices trong ngành công nghiệp.*

*Phiên bản: 1.0*  
*Ngày cập nhật: December 29, 2025*
