#ifndef CLIENT_METRICS_H
#define CLIENT_METRICS_H

#include <string>
#include <chrono>
#include <atomic>

struct ClientMetrics {
    double rtt_ms = 0.0;               // Round-trip time in milliseconds
    double throughput_kbps = 0.0;      // Throughput in kilobits per second
    double packet_loss_rate = 0.0;     // Packet loss percentage
    double transfer_latency_ms = 0.0;   // Transfer latency in milliseconds
    
    // For packet loss calculation
    std::atomic<uint64_t> total_requests{0};
    std::atomic<uint64_t> failed_requests{0};
    
    // Custom copy constructor and assignment for atomic members
    ClientMetrics() = default;
    
    ClientMetrics(const ClientMetrics& other) 
        : rtt_ms(other.rtt_ms),
          throughput_kbps(other.throughput_kbps),
          packet_loss_rate(other.packet_loss_rate),
          transfer_latency_ms(other.transfer_latency_ms),
          total_requests(other.total_requests.load()),
          failed_requests(other.failed_requests.load()) {
    }
    
    ClientMetrics& operator=(const ClientMetrics& other) {
        if (this != &other) {
            rtt_ms = other.rtt_ms;
            throughput_kbps = other.throughput_kbps;
            packet_loss_rate = other.packet_loss_rate;
            transfer_latency_ms = other.transfer_latency_ms;
            total_requests.store(other.total_requests.load());
            failed_requests.store(other.failed_requests.load());
        }
        return *this;
    }

    void log_csv(const std::string &filename) const; 
};

#endif 