# File Transfer Client - Architecture Diagrams

## System Architecture

```
┌───────────────────────────────────────────────────────────────┐
│                        User Application                        │
│                    (client_app.cpp or custom)                  │
└─────────────────────────┬─────────────────────────────────────┘
                          │
                          │ uses
                          ▼
┌───────────────────────────────────────────────────────────────┐
│                      Client Interface                         │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  + connect(ip, port): bool                              │ │
│  │  + uploadFile(path): bool                               │ │
│  │  + downloadFile(name, dir): bool                        │ │
│  │  + listFiles(): vector<string>                          │ │
│  │  + getMetrics(): TransferMetrics                        │ │
│  │  + setProgressCallback(callback)                        │ │
│  └─────────────────────────────────────────────────────────┘ │
└─────────────────────────┬─────────────────────────────────────┘
                          │
                          │ implements
                          ▼
┌───────────────────────────────────────────────────────────────┐
│                     Client Implementation                      │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │  Private Members:                                       │ │
│  │  - socket_: unique_ptr<ClientSocket>                    │ │
│  │  - protocol_: unique_ptr<ClientProtocol>                │ │
│  │  - metricsCollector_: unique_ptr<MetricsCollector>      │ │
│  │  - coreMetrics_: unique_ptr<ClientMetrics>              │ │
│  │  - chunkSize_: size_t                                   │ │
│  │  - connected_: bool                                     │ │
│  │  - callbacks: progress, error                           │ │
│  └─────────────────────────────────────────────────────────┘ │
└──┬──────────────┬──────────────┬──────────────┬──────────────┘
   │              │              │              │
   │              │              │              │
   ▼              ▼              ▼              ▼
┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐
│  Socket  │  │ Protocol │  │ Metrics  │  │   Core   │
│  Layer   │  │  Layer   │  │Collector │  │ Metrics  │
└──────────┘  └──────────┘  └──────────┘  └──────────┘
     │              │              │              │
     │              │              │              │
     ▼              ▼              ▼              ▼
┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐
│   TCP    │  │ Messages │  │Transfer  │  │   RTT    │
│  Socket  │  │Formatting│  │  Stats   │  │Throughput│
└──────────┘  └──────────┘  └──────────┘  └──────────┘
```

## Component Interaction - Upload Flow

```
┌─────────┐
│  User   │
└────┬────┘
     │ 1. uploadFile("file.txt")
     ▼
┌─────────────────┐
│     Client      │
│  ┌───────────┐  │
│  │Check      │  │
│  │Connection │  │
│  └─────┬─────┘  │
│        │        │
│  ┌─────▼─────┐  │
│  │Start      │  │
│  │Metrics    │  │
│  └─────┬─────┘  │
│        │        │
│  ┌─────▼─────┐  │
│  │Protocol   │  │
│  │request_put│  │
│  └─────┬─────┘  │
└────────┼────────┘
         │
    ┌────▼─────────────────────────────┐
    │  sendFileChunked()               │
    │  ┌────────────────────────────┐  │
    │  │  Loop for each chunk:      │  │
    │  │  1. Read chunk from file   │  │
    │  │  2. Send via socket        │  │
    │  │  3. Update metrics         │  │
    │  │  4. Call progress callback │  │
    │  │  5. Record packet sent     │  │
    │  └────────────────────────────┘  │
    └───────────────┬──────────────────┘
                    │
    ┌───────────────▼──────────────────┐
    │  Complete Transfer                │
    │  1. Calculate final metrics       │
    │  2. Update statistics             │
    │  3. Return success/failure        │
    └───────────────────────────────────┘
```

## Component Interaction - Download Flow

```
┌─────────┐
│  User   │
└────┬────┘
     │ 1. downloadFile("remote.txt", "./")
     ▼
┌─────────────────┐
│     Client      │
│  ┌───────────┐  │
│  │Check      │  │
│  │Connection │  │
│  └─────┬─────┘  │
│        │        │
│  ┌─────▼─────┐  │
│  │Start      │  │
│  │Metrics    │  │
│  └─────┬─────┘  │
│        │        │
│  ┌─────▼─────┐  │
│  │Protocol   │  │
│  │request_get│  │
│  └─────┬─────┘  │
└────────┼────────┘
         │
    ┌────▼─────────────────────────────┐
    │  receiveFileChunked()            │
    │  ┌────────────────────────────┐  │
    │  │  Loop until complete:      │  │
    │  │  1. Receive chunk          │  │
    │  │  2. Write to file          │  │
    │  │  3. Update metrics         │  │
    │  │  4. Call progress callback │  │
    │  │  5. Record packet received │  │
    │  └────────────────────────────┘  │
    └───────────────┬──────────────────┘
                    │
    ┌───────────────▼──────────────────┐
    │  Complete Transfer                │
    │  1. Close file                    │
    │  2. Calculate final metrics       │
    │  3. Return success/failure        │
    └───────────────────────────────────┘
```

## Metrics Collection Flow

```
┌────────────────────────────────────────────┐
│         MetricsCollector                   │
├────────────────────────────────────────────┤
│                                            │
│  startTransfer(totalSize, totalChunks)    │
│         │                                  │
│         ▼                                  │
│  ┌───────────────────────────────┐        │
│  │ Initialize:                   │        │
│  │ - transferStartTime           │        │
│  │ - totalTransferSize           │        │
│  │ - totalChunks                 │        │
│  └───────────┬───────────────────┘        │
│              │                             │
│  ┌───────────▼───────────────────┐        │
│  │ updateTransfer(bytes)         │ ◄──────┼── Called for each chunk
│  │   │                           │        │
│  │   ├─► calculateSpeed()        │        │
│  │   ├─► calculateProgress()     │        │
│  │   └─► calculateETA()          │        │
│  └───────────┬───────────────────┘        │
│              │                             │
│              ├─► Notify progress callback  │
│              └─► Update connection stats   │
│                                            │
│  completeTransfer()                       │
│         │                                  │
│         ▼                                  │
│  ┌───────────────────────────────┐        │
│  │ Finalize:                     │        │
│  │ - Set progress = 100%         │        │
│  │ - Calculate avg speed         │        │
│  │ - Set ETA = 0                 │        │
│  └───────────────────────────────┘        │
└────────────────────────────────────────────┘
```

## Class Diagram

```
┌─────────────────────────────────────┐
│          <<interface>>              │
│             Client                 │
├─────────────────────────────────────┤
│ + connect(string, uint16_t): bool  │
│ + disconnect(): void                │
│ + isConnected(): bool               │
│ + uploadFile(string): bool          │
│ + downloadFile(string, string): bool│
│ + listFiles(): vector<string>       │
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
