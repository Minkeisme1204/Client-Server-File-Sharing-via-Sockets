# Client-Server File Transfer System - Architecture Diagrams

## System Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                      CLIENT APPLICATION                         │
│                    (client_test.cpp / API)                      │
└───────────────────────┬─────────────────────────────────────────┘
                        │
                        │ uses
                        ▼
┌────────────────────────────────────┐     ┌────────────────────────────────────┐
│         CLIENT SIDE                │     │         SERVER SIDE                │
│  ┌──────────────────────────────┐  │     │  ┌──────────────────────────────┐  │
│  │         Client               │  │     │  │         Server               │  │
│  ├──────────────────────────────┤  │     │  ├──────────────────────────────┤  │
│  │ - socket_: ClientSocket      │  │     │  │ - socket_: ServerSocket      │  │
│  │ - protocol_: ClientProtocol  │  │     │  │ - protocol_: ServerProtocol  │  │
│  │ - metrics_: ClientMetrics    │  │     │  │ - metrics_: ServerMetrics    │  │
│  │ - timeout_: int              │  │     │  │ - sessions_: vector<Session> │  │
│  │ - verbose_: bool             │  │     │  │ - sharedDir_: shared_ptr<str>│  │
│  └──────────────┬───────────────┘  │     │  │ - maxConnections_: size_t    │  │
│                 │                   │     │  │ - running_: atomic<bool>     │  │
│                 │                   │     │  └──────────────┬───────────────┘  │
│  ┌──────────────▼───────────────┐  │     │  ┌──────────────▼───────────────┐  │
│  │     ClientSocket             │  │◄────┼──┤     ServerSocket             │  │
│  ├──────────────────────────────┤  │ TCP │  ├──────────────────────────────┤  │
│  │ - sockfd_: int               │  │     │  │ - sockfd_: int               │  │
│  │ - connected_: bool           │  │     │  │ - bound_: bool               │  │
│  │ + connectToServer()          │  │     │  │ + bind(port)                 │  │
│  │ + sendData()                 │  │     │  │ + acceptConnection()         │  │
│  │ + receiveData()              │  │     │  │ + static sendData()          │  │
│  └──────────────┬───────────────┘  │     │  │ + static receiveData()       │  │
│                 │                   │     │  └──────────────┬───────────────┘  │
│  ┌──────────────▼───────────────┐  │     │  ┌──────────────▼───────────────┐  │
│  │    ClientProtocol            │  │     │  │    ServerProtocol            │  │
│  ├──────────────────────────────┤  │     │  ├──────────────────────────────┤  │
│  │ - socket_: ClientSocket&     │  │     │  │ - sharedDirectory_: shared_ptr│ │
│  │ - metrics_: ClientMetrics*   │  │     │  │ - metrics_: ServerMetrics*   │  │
│  │ + request_list()             │  │     │  │ + processRequest(clientFd)   │  │
│  │ + request_get(filename)      │  │     │  │ + handleListCommand()        │  │
│  │ + request_put(filepath)      │  │     │  │ + handleGetCommand()         │  │
│  │ + setMetrics()               │  │     │  │ + handlePutCommand()         │  │
│  └──────────────┬───────────────┘  │     │  │ + setMetrics()               │  │
│                 │                   │     │  │ + setSharedDirectoryPtr()    │  │
│  ┌──────────────▼───────────────┐  │     │  └──────────────┬───────────────┘  │
│  │     ClientMetrics            │  │     │                 │                   │
│  ├──────────────────────────────┤  │     │  ┌──────────────▼───────────────┐  │
│  │ - rtt_ms: double             │  │     │  │    ClientSession             │  │
│  │ - throughput_kbps: double    │  │     │  ├──────────────────────────────┤  │
│  │ - packet_loss_rate: double   │  │     │  │ - clientFd_: int             │  │
│  │ - transfer_latency_ms: double│  │     │  │ - sharedDir_: shared_ptr<str>│  │
│  │ - total_requests: atomic     │  │     │  │ - metrics_: ServerMetrics*   │  │
│  │ - failed_requests: atomic    │  │     │  │ - thread_: unique_ptr        │  │
│  │ + log_csv()                  │  │     │  │ - active_: atomic<bool>      │  │
│  └──────────────────────────────┘  │     │  │ + start()                    │  │
│                                     │     │  │ + stop()                     │  │
└─────────────────────────────────────┘     │  │ + handleSession()            │  │
                                            │  └──────────────┬───────────────┘  │
                                            │                 │                   │
                                            │  ┌──────────────▼───────────────┐  │
                                            │  │    ServerMetrics             │  │
                                            │  ├──────────────────────────────┤  │
                                            │  │ - totalConnections: atomic   │  │
                                            │  │ - activeConnections: atomic  │  │
                                            │  │ - totalBytesReceived: atomic │  │
                                            │  │ - totalBytesSent: atomic     │  │
                                            │  │ - filesUploaded: atomic      │  │
                                            │  │ - filesDownloaded: atomic    │  │
                                            │  │ - averageThroughput_kbps     │  │
                                            │  │ - peakThroughput_kbps        │  │
                                            │  │ - averageLatency_ms          │  │
                                            │  │ + updateThroughput()         │  │
                                            │  │ + updateLatency()            │  │
                                            │  │ + addBytesReceived()         │  │
                                            │  │ + addBytesSent()             │  │
                                            │  └──────────────────────────────┘  │
                                            └─────────────────────────────────────┘
```


## Protocol Commands Flow

```
CLIENT                                    SERVER
  │                                         │
  │  ┌──────────────────────────────┐      │
  │  │ CMD_LIST (0x01)              │      │
  ├──┼──────────────────────────────┼──────►│
  │  └──────────────────────────────┘      │ ┌────────────────────────┐
  │                                         │ │ 1. Read shared dir     │
  │                                         │ │ 2. List files          │
  │                                         │ │ 3. Count files         │
  │  ┌──────────────────────────────┐      │ └────────────────────────┘
  │  │ fileCount (uint32_t)         │      │
  │◄─┼──────────────────────────────┼──────┤
  │  │ filename[256] × N            │      │
  │◄─┼──────────────────────────────┼──────┤
  │  └──────────────────────────────┘      │
  │                                         │
  │  ┌──────────────────────────────┐      │
  │  │ CMD_GET (0x02)               │      │
  ├──┼──────────────────────────────┼──────►│
  │  │ filename[256]                │      │
  ├──┼──────────────────────────────┼──────►│ ┌────────────────────────┐
  │  └──────────────────────────────┘      │ │ 1. Validate filename   │
  │                                         │ │ 2. stat() for size     │
  │  ┌──────────────────────────────┐      │ │ 3. Open file           │
  │  │ fileSize (uint64_t)          │      │ └────────────────────────┘
  │◄─┼──────────────────────────────┼──────┤
  │  │ fileData (chunks of 8KB)     │      │ ┌────────────────────────┐
  │◄─┼──────────────────────────────┼──────┤ │ Loop: read & send      │
  │  └──────────────────────────────┘      │ │ Update metrics         │
  │                                         │ └────────────────────────┘
  │  ┌──────────────────────────────┐      │
  │  │ CMD_PUT (0x03)               │      │
  ├──┼──────────────────────────────┼──────►│
  │  │ filename[256]                │      │
  ├──┼──────────────────────────────┼──────►│
  │  │ fileSize (uint64_t)          │      │
  ├──┼──────────────────────────────┼──────►│ ┌────────────────────────┐
  │  │ fileData (chunks of 8KB)     │      │ │ 1. Create file         │
  ├──┼──────────────────────────────┼──────►│ │ 2. Loop: receive & write│
  │  └──────────────────────────────┘      │ │ 3. Update metrics      │
  │                                         │ └────────────────────────┘
  │                                         │
```

## Server Multi-Session Architecture

```
┌──────────────────────────────────────────────────────────────────┐
│                          Server::run()                           │
│                                                                  │
│  running_ = true                                                │
│  ┌────────────────────────────────────────────────────────┐    │
│  │             acceptLoop() - Main Thread                  │    │
│  │                                                         │    │
│  │  while (running_) {                                    │    │
│  │    ┌──────────────────────────────────┐               │    │
│  │    │ cleanupFinishedSessions()        │               │    │
│  │    └──────────────────────────────────┘               │    │
│  │    ┌──────────────────────────────────┐               │    │
│  │    │ Check maxConnections limit       │               │    │
│  │    └──────────────────────────────────┘               │    │
│  │    ┌──────────────────────────────────┐               │    │
│  │    │ clientFd = socket_->accept()     │               │    │
│  │    └──────┬───────────────────────────┘               │    │
│  │           │                                            │    │
│  │           ▼                                            │    │
│  │    ┌──────────────────────────────────┐               │    │
│  │    │ new ClientSession(               │               │    │
│  │    │   clientFd,                      │               │    │
│  │    │   clientAddr,                    │               │    │
│  │    │   sharedDirectory_,              │ ◄──shared_ptr │    │
│  │    │   &metrics_)                     │               │    │
│  │    └──────┬───────────────────────────┘               │    │
│  │           │                                            │    │
│  │           │ session->start()                          │    │
│  │           ▼                                            │    │
│  │    ┌──────────────────────────────────┐               │    │
│  │    │ sessions_.push_back(session)     │               │    │
│  │    └──────────────────────────────────┘               │    │
│  │  }                                                     │    │
│  └────────────────────────────────────────────────────────┘    │
└──────────────────────────────────────────────────────────────────┘
                          │
                          │ spawns
                          ▼
┌──────────────────────────────────────────────────────────────────┐
│              ClientSession (One per client)                      │
│  ┌────────────────────────────────────────────────────────┐     │
│  │           handleSession() - Separate Thread            │     │
│  │                                                         │     │
│  │  Create ServerProtocol instance                        │     │
│  │  protocol.setSharedDirectoryPtr(sharedDir_)           │     │
│  │  protocol.setMetrics(metrics_)                        │     │
│  │                                                         │     │
│  │  while (active_) {                                     │     │
│  │    ┌──────────────────────────────────┐               │     │
│  │    │ protocol.processRequest(clientFd)│               │     │
│  │    └──────┬───────────────────────────┘               │     │
│  │           │                                            │     │
│  │           ├─► handleListCommand()                      │     │
│  │           ├─► handleGetCommand()                       │     │
│  │           │    ├─ sendFile()                          │     │
│  │           │    ├─ metrics_->addBytesSent()           │     │
│  │           │    └─ metrics_->filesDownloaded++        │     │
│  │           └─► handlePutCommand()                       │     │
│  │                ├─ receiveFile()                       │     │
│  │                ├─ metrics_->addBytesReceived()       │     │
│  │                └─ metrics_->filesUploaded++          │     │
│  │  }                                                     │     │
│  │                                                         │     │
│  │  cleanup() - close(clientFd_)                         │     │
│  └────────────────────────────────────────────────────────┘     │
└──────────────────────────────────────────────────────────────────┘
```


## Dynamic Shared Directory Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                         Server                                  │
│  ┌───────────────────────────────────────────────────────┐     │
│  │  sharedDirectory_: std::shared_ptr<std::string>       │     │
│  │     │                                                  │     │
│  │     │  Points to: "./shared"                          │     │
│  │     │                                                  │     │
│  │     └──────┬───────────────────────────────┬─────────►│     │
│  │            │                                │          │     │
│  └────────────┼────────────────────────────────┼──────────┘     │
│               │                                │                │
│               │ Pass shared_ptr                │ Pass shared_ptr│
│               │ to new sessions                │ to protocol    │
│               │                                │                │
└───────────────┼────────────────────────────────┼────────────────┘
                │                                │
                ▼                                ▼
┌─────────────────────────────┐  ┌─────────────────────────────┐
│     ClientSession 1         │  │     ClientSession 2         │
│  ┌───────────────────────┐  │  │  ┌───────────────────────┐  │
│  │ sharedDir_: shared_ptr│──┼──┼──┼─►Same pointer          │  │
│  │    │                  │  │  │  │    │                  │  │
│  │    ▼                  │  │  │  │    ▼                  │  │
│  │ ServerProtocol        │  │  │  │ ServerProtocol        │  │
│  │  sharedDirectory_:    │  │  │  │  sharedDirectory_:    │  │
│  │    shared_ptr ────────┼──┼──┼──┼────►shared_ptr        │  │
│  └───────────────────────┘  │  │  └───────────────────────┘  │
└─────────────────────────────┘  └─────────────────────────────┘
                │                                │
                └────────────┬───────────────────┘
                             │
                             ▼
                    ┌────────────────────┐
                    │  Shared String     │
                    │  "./shared"        │
                    │                    │
                    │  When Server       │
                    │  calls:            │
                    │  *sharedDirectory_ │
                    │    = "new/path"    │
                    │                    │
                    │  ALL sessions      │
                    │  automatically see │
                    │  new directory!    │
                    └────────────────────┘

KEY BENEFIT: Dynamic directory change affects all active sessions instantly!
```

## Metrics Tracking Flow

```
CLIENT REQUEST                    SERVER PROCESSING               METRICS UPDATE
     │                                   │                             │
     │  CMD_PUT                          │                             │
     ├───────────────────────────────────►│                             │
     │  filename + size                  │                             │
     ├───────────────────────────────────►│                             │
     │                                   │                             │
     │  file data chunks                 │                             │
     ├───────────────────────────────────►│                             │
     │                                   │                             │
     │                                   │  receiveFile()              │
     │                                   │  ┌──────────────────┐       │
     │                                   │  │ Loop receiving   │       │
     │                                   │  │ totalReceived += │       │
     │                                   │  │   bytes          │       │
     │                                   │  └──────────────────┘       │
     │                                   │                             │
     │                                   │  if (metrics_) {            │
     │                                   ├─────────────────────────────►│
     │                                   │   addBytesReceived()        │
     │                                   │                             │ ┌──────────────┐
     │                                   │   filesUploaded++           │ │ totalBytes   │
     │                                   │                             │ │ Received += N│
     │                                   │                             │ └──────────────┘
     │                                   │  }                          │
     │                                   │                             │ ┌──────────────┐
     │                                   │                             │ │ filesUploaded│
     │                                   │                             │ │     += 1     │
     │                                   │                             │ └──────────────┘
     │  CMD_GET                          │                             │
     ├───────────────────────────────────►│                             │
     │  filename                         │                             │
     ├───────────────────────────────────►│                             │
     │                                   │                             │
     │  ◄────fileSize                    │                             │
     │  ◄────file data                   │  sendFile()                 │
     │                                   │  ┌──────────────────┐       │
     │                                   │  │ Loop sending     │       │
     │                                   │  │ totalSent += bytes│      │
     │                                   │  └──────────────────┘       │
     │                                   │                             │
     │                                   │  if (metrics_) {            │
     │                                   ├─────────────────────────────►│
     │                                   │   addBytesSent()            │
     │                                   │                             │ ┌──────────────┐
     │                                   │   filesDownloaded++         │ │ totalBytes   │
     │                                   │                             │ │   Sent += N  │
     │                                   │                             │ └──────────────┘
     │                                   │  }                          │
     │                                   │                             │ ┌──────────────┐
     │                                   │                             │ │filesDownload │
     │                                   │                             │ │     += 1     │
     │                                   │                             │ └──────────────┘
     │                                   │                             │
     │                                   │  processRequest() completes │
     │                                   │  Duration = end - start     │
     │                                   ├─────────────────────────────►│
     │                                   │   updateLatency(duration)   │
     │                                   │                             │ ┌──────────────┐
     │                                   │                             │ │ averageLatency│
     │                                   │                             │ │ = EMA(new)   │
     │                                   │                             │ └──────────────┘

All metrics are thread-safe using std::atomic or mutex protection!
```


## Client Metrics Calculation

```
┌───────────────────────────────────────────────────────────────┐
│                    Client Operations                          │
│                                                               │
│  listFiles() / getFile() / putFile()                         │
│  ┌────────────────────────────────────────────────────┐      │
│  │  metrics_.total_requests++                         │      │
│  │  start = high_resolution_clock::now()              │      │
│  │                                                     │      │
│  │  protocol_->request_XXX(...)                       │      │
│  │                                                     │      │
│  │  end = high_resolution_clock::now()                │      │
│  │  duration_us = duration_cast<microseconds>(end-start) │   │
│  │  duration_ms = duration_us.count() / 1000.0        │      │
│  │                                                     │      │
│  │  // Exponential Moving Average for RTT             │      │
│  │  if (rtt_ms == 0.0) {                              │      │
│  │    rtt_ms = duration_ms                            │      │
│  │  } else {                                           │      │
│  │    rtt_ms = (rtt_ms * 0.7) + (duration_ms * 0.3)  │      │
│  │  }                                                  │      │
│  │                                                     │      │
│  │  transfer_latency_ms = duration_ms                 │      │
│  └────────────────────────────────────────────────────┘      │
│                                                               │
│  On Success: metrics updated                                 │
│  On Failure: metrics_.failed_requests++                      │
│                                                               │
│  Display Metrics:                                            │
│  ┌────────────────────────────────────────────────────┐      │
│  │  packet_loss = (failed_requests / total_requests)  │      │
│  │                × 100%                              │      │
│  │                                                     │      │
│  │  RTT:                rtt_ms                        │      │
│  │  Throughput:         throughput_kbps               │      │
│  │  Packet Loss:        packet_loss                   │      │
│  │  Transfer Latency:   transfer_latency_ms           │      │
│  └────────────────────────────────────────────────────┘      │
└───────────────────────────────────────────────────────────────┘

Throughput Calculation (in ClientProtocol):
┌────────────────────────────────────────────────────────────┐
│  Throughput_kbps = (Bytes × 8) / (Duration_ms / 1000) / 1024 │
│                                                            │
│  Example: 2MB file in 1000ms                              │
│  = (2,097,152 × 8) / 1 / 1024                            │
│  = 16,384 kbps                                            │
└────────────────────────────────────────────────────────────┘
```

## Server Metrics Calculation

```
┌───────────────────────────────────────────────────────────────┐
│                  ServerMetrics Updates                        │
│                                                               │
│  1. Connection Management:                                    │
│  ┌────────────────────────────────────────────────────┐      │
│  │  incrementConnections()                            │      │
│  │    totalConnections++  (atomic)                    │      │
│  │    activeConnections++ (atomic)                    │      │
│  │                                                     │      │
│  │  decrementActiveConnections()                      │      │
│  │    activeConnections-- (atomic)                    │      │
│  └────────────────────────────────────────────────────┘      │
│                                                               │
│  2. Bytes Tracking:                                          │
│  ┌────────────────────────────────────────────────────┐      │
│  │  addBytesReceived(bytes)                           │      │
│  │    totalBytesReceived += bytes (atomic)            │      │
│  │                                                     │      │
│  │  addBytesSent(bytes)                               │      │
│  │    totalBytesSent += bytes (atomic)                │      │
│  └────────────────────────────────────────────────────┘      │
│                                                               │
│  3. Throughput Calculation:                                  │
│  ┌────────────────────────────────────────────────────┐      │
│  │  updateThroughput(bytes, duration_ms)              │      │
│  │    throughput = (bytes × 8) / (duration_ms/1000) / 1024│  │
│  │                                                     │      │
│  │    // Exponential Moving Average                   │      │
│  │    if (averageThroughput == 0.0) {                 │      │
│  │      averageThroughput = throughput                │      │
│  │    } else {                                         │      │
│  │      averageThroughput = avg*0.9 + throughput*0.1  │      │
│  │    }                                                │      │
│  │                                                     │      │
│  │    if (throughput > peakThroughput) {              │      │
│  │      peakThroughput = throughput                   │      │
│  │    }                                                │      │
│  └────────────────────────────────────────────────────┘      │
│                                                               │
│  4. Latency Calculation:                                     │
│  ┌────────────────────────────────────────────────────┐      │
│  │  updateLatency(latency_ms)                         │      │
│  │    // Exponential Moving Average                   │      │
│  │    if (averageLatency == 0.0) {                    │      │
│  │      averageLatency = latency_ms                   │      │
│  │    } else {                                         │      │
│  │      averageLatency = avg*0.9 + latency_ms*0.1     │      │
│  │    }                                                │      │
│  └────────────────────────────────────────────────────┘      │
│                                                               │
│  5. Files Counter:                                           │
│  ┌────────────────────────────────────────────────────┐      │
│  │  filesUploaded++   (atomic, in receiveFile)        │      │
│  │  filesDownloaded++ (atomic, in sendFile)           │      │
│  └────────────────────────────────────────────────────┘      │
└───────────────────────────────────────────────────────────────┘
```
│ + deleteFile(string): bool          │
│ + getTransferMetrics(): Metrics     │
│ + setProgressCallback(func): void   │
└───────────────┬─────────────────────┘
                │
                │ implements
                ▼
┌─────────────────────────────────────┐
│             Client                  │
├─────────────────────────────────────┤
│ - socket_: unique_ptr<ClientSocket> │
│ - protocol_: unique_ptr<Protocol>   │
│ - metrics_: unique_ptr<Metrics>     │
│ - chunkSize_: size_t                │
│ - connected_: bool                  │
│ - progressCallback_: function       │
│ - errorCallback_: function          │
├─────────────────────────────────────┤
│ + Client()                          │
│ + ~Client()                         │
│ + connect(...): bool                │
│ + uploadFile(...): bool             │
│ - sendFileChunked(...): bool        │
│ - receiveFileChunked(...): bool     │
│ - notifyProgress(...): void         │
└──┬──────────┬──────────┬───────────┘
   │          │          │
   │          │          │
   │ has-a    │ has-a    │ has-a
   │          │          │
   ▼          ▼          ▼
┌────────┐ ┌────────┐ ┌────────────┐
│ Socket │ │Protocol│ │MetricsCol- │
│        │ │        │ │  lector    │
└────────┘ └────────┘ └────────────┘
```

## Callback Flow

```
┌─────────────────────────────────────────────┐
│           User Application                  │
│                                             │
│  client->setProgressCallback(               │
│      [](double prog, uint64_t bytes) {     │
│          updateUI(prog, bytes);            │
│      }                                      │
│  );                                         │
│                                             │
│  client->setErrorCallback(                  │
│      [](const string& err) {               │
│          logError(err);                    │
│      }                                      │
│  );                                         │
└──────────────┬──────────────────────────────┘
               │
               │ stores callbacks
               ▼
┌─────────────────────────────────────────────┐
│              Client                         │
│  ┌─────────────────────────────────────┐   │
│  │ progressCallback_: function         │   │
│  │ errorCallback_: function            │   │
│  └─────────────────────────────────────┘   │
│                                             │
│  During transfer:                           │
│  ┌─────────────────────────────────────┐   │
│  │ updateTransfer()                    │   │
│  │   ├─> Calculate progress            │   │
│  │   └─> notifyProgress()              │───┼─┐
│  └─────────────────────────────────────┘   │ │
│                                             │ │
│  On error:                                  │ │
│  ┌─────────────────────────────────────┐   │ │
│  │ setError()                          │   │ │
│  │   └─> notifyError()                 │───┼─┤
│  └─────────────────────────────────────┘   │ │
└─────────────────────────────────────────────┘ │
                                                │
               ┌────────────────────────────────┘
               │
               │ callbacks invoked
               ▼
┌─────────────────────────────────────────────┐
│       User-defined Callback Functions       │
│  ┌─────────────────────────────────────┐   │
│  │ updateUI(progress, bytes)           │   │
│  │   - Update progress bar             │   │
│  │   - Show transfer speed             │   │
│  │   - Display ETA                     │   │
│  └─────────────────────────────────────┘   │
│                                             │
│  ┌─────────────────────────────────────┐   │
│  │ logError(error)                     │   │
│  │   - Write to log file               │   │
│  │   - Show error message              │   │
│  │   - Update status                   │   │
│  └─────────────────────────────────────┘   │
└─────────────────────────────────────────────┘
```

## State Machine - Connection

```
        ┌─────────────┐
        │             │
        │ Disconnected│◄───────────────┐
        │             │                 │
        └──────┬──────┘                 │
               │                        │
               │ connect()              │
               ▼                        │
        ┌─────────────┐                 │
        │             │                 │
        │ Connecting  │                 │
        │             │                 │
        └──┬──────┬───┘                 │
           │      │                     │
  success  │      │ failure             │
           │      └─────────────────────┘
           ▼
        ┌─────────────┐
        │             │
        │  Connected  │
        │             │
        └──────┬──────┘
               │
               │ disconnect() or error
               ▼
        ┌─────────────┐
        │             │
        │Disconnecting│
        │             │
        └──────┬──────┘
               │
               └───────────────────┐
                                   │
                                   ▼
                            ┌─────────────┐
                            │             │
                            │ Disconnected│
                            │             │
                            └─────────────┘
```

## Data Flow - Chunked Transfer

```
┌──────────────┐
│  Source File │
└──────┬───────┘
       │
       │ Read chunk
       ▼
┌──────────────────────────────┐
│  Chunk Buffer                │
│  [═══════════════════]       │
│  64KB (configurable)         │
└──────┬───────────────────────┘
       │
       │ Send via socket
       ▼
┌──────────────────────────────┐
│  Network (TCP)               │
│  ════════════════════>       │
└──────┬───────────────────────┘
       │
       │ Receive
       ▼
┌──────────────────────────────┐
│  Receive Buffer              │
│  [═══════════════════]       │
└──────┬───────────────────────┘
       │
       │ Write chunk
       ▼
┌──────────────┐
│Destination   │
│    File      │
└──────────────┘

       ║
       ║ Parallel tracking
       ▼
┌──────────────────────────────┐
│  MetricsCollector            │
│  - Bytes transferred         │
│  - Transfer speed            │
│  - Progress %                │
│  - ETA                       │
└──────────────────────────────┘
```

---

These diagrams illustrate the clean, modular architecture of the File Transfer Client with clear separation of concerns and well-defined component interactions.
