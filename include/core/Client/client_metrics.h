#ifndef CLIENT_METRICS_H
#define CLIENT_METRICS_H

#include <string>
#include <chrono> 

struct ClientMetrics {
    double rtt_ms = 0.0;               // Round-trip time in milliseconds
    double throughput_kbps = 0.0;      // Throughput in kilobits per second
    double packet_loss_rate = 0.0;     // Packet loss percentage
    double transfer_latency_ms = 0.0;   // Transfer latency in milliseconds
    
    // Additional tracking
    uint64_t totalBytesSent = 0;
    uint64_t totalBytesReceived = 0;
    uint32_t filesUploaded = 0;
    uint32_t filesDownloaded = 0;
    uint32_t totalOperations = 0;
    uint32_t failedOperations = 0;

    void log_csv(const std::string &filename) const; 
};

#endif 