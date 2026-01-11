#ifndef CLIENT_METRICS_H
#define CLIENT_METRICS_H

#include <string>
#include <chrono>
#include <atomic>
#include <vector>
#include <ctime>

/**
 * @struct RequestRecord
 * @brief Record of a single client request
 */
struct RequestRecord {
    std::time_t timestamp;           // When the request was made
    std::string operation;           // GET, PUT, LIST
    std::string filename;            // File name (empty for LIST)
    bool success;                    // Whether request succeeded
    uint64_t bytes_transferred;      // Bytes transferred
    double duration_ms;              // Request duration in milliseconds
    std::string error_msg;           // Error message if failed
    
    RequestRecord(const std::string& op, const std::string& file, bool succ, 
                  uint64_t bytes, double dur, const std::string& err = "")
        : timestamp(std::time(nullptr)), operation(op), filename(file),
          success(succ), bytes_transferred(bytes), duration_ms(dur), error_msg(err) {}
};

struct ClientMetrics {
    double rtt_ms = 0.0;               // Round-trip time in milliseconds
    double throughput_kbps = 0.0;      // Throughput in kilobits per second
    double packet_loss_rate = 0.0;     // Packet loss percentage
    double transfer_latency_ms = 0.0;   // Transfer latency in milliseconds
    
    // For packet loss calculation
    std::atomic<uint64_t> total_requests{0};
    std::atomic<uint64_t> failed_requests{0};
    
    // For throughput calculation
    std::atomic<uint64_t> total_bytes_sent{0};        // Total bytes uploaded (PUT)
    std::atomic<uint64_t> total_bytes_received{0};    // Total bytes downloaded (GET)
    std::atomic<uint64_t> total_transfer_time_ms{0};
    
    // Request history
    std::vector<RequestRecord> request_history;
    
    // Custom copy constructor and assignment for atomic members
    ClientMetrics() = default;
    
    ClientMetrics(const ClientMetrics& other) 
        : rtt_ms(other.rtt_ms),
          throughput_kbps(other.throughput_kbps),
          packet_loss_rate(other.packet_loss_rate),
          transfer_latency_ms(other.transfer_latency_ms),
          total_requests(other.total_requests.load()),
          failed_requests(other.failed_requests.load()),
          total_bytes_sent(other.total_bytes_sent.load()),
          total_bytes_received(other.total_bytes_received.load()),
          total_transfer_time_ms(other.total_transfer_time_ms.load()),
          request_history(other.request_history) {
    }
    
    ClientMetrics& operator=(const ClientMetrics& other) {
        if (this != &other) {
            rtt_ms = other.rtt_ms;
            throughput_kbps = other.throughput_kbps;
            packet_loss_rate = other.packet_loss_rate;
            transfer_latency_ms = other.transfer_latency_ms;
            total_requests.store(other.total_requests.load());
            failed_requests.store(other.failed_requests.load());
            total_bytes_sent.store(other.total_bytes_sent.load());
            total_bytes_received.store(other.total_bytes_received.load());
            total_transfer_time_ms.store(other.total_transfer_time_ms.load());
            request_history = other.request_history;
        }
        return *this;
    }

    void log_csv(const std::string &filename) const; 
};

#endif 