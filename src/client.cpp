#include "client.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstring>

// Constructor
Client::Client() 
    : socket_(std::make_unique<ClientSocket>()),
      protocol_(nullptr),
      metrics_{},
      timeout_(30),
      verbose_(false) {
}

// Destructor
Client::~Client() {
    disconnect();
}

// Connection Management
bool Client::connect(const std::string& ip, uint16_t port) {
    if (verbose_) {
        std::cout << "[Client] Connecting to " << ip << ":" << port << "...\n";
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    
    bool success = socket_->connectToServer(ip, port);
    
    if (success) {
        // Initialize protocol after successful connection
        protocol_ = std::make_unique<ClientProtocol>(*socket_);
        protocol_->setMetrics(&metrics_);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        metrics_.rtt_ms = duration.count();
        
        if (verbose_) {
            std::cout << "[Client] Connected successfully (RTT: " << metrics_.rtt_ms << " ms)\n";
        }
        
        logOperation("connect", true);
    } else {
        std::cerr << "[Client] Failed to connect to " << ip << ":" << port << "\n";
        logOperation("connect", false);
    }
    
    return success;
}

void Client::disconnect() {
    if (socket_ && socket_->isConnected()) {
        socket_->disconnect();
        protocol_.reset();
        
        if (verbose_) {
            std::cout << "[Client] Disconnected from server\n";
        }
        
        logOperation("disconnect", true);
    }
}

bool Client::isConnected() const {
    return socket_ && socket_->isConnected();
}

// File Operations
bool Client::listFiles() {
    if (!isConnected()) {
        std::cerr << "[Client] Not connected to server\n";
        return false;
    }

    if (verbose_) {
        std::cout << "[Client] Requesting file list...\n";
    }

    try {
        metrics_.total_requests++;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        protocol_->request_list();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        double duration_ms = duration_us.count() / 1000.0;
        
        // Update RTT with exponential moving average
        if (metrics_.rtt_ms == 0.0) {
            metrics_.rtt_ms = duration_ms;
        } else {
            metrics_.rtt_ms = (metrics_.rtt_ms * 0.7) + (duration_ms * 0.3);
        }
        metrics_.transfer_latency_ms = duration_ms;
        
        logOperation("list", true);
        return true;
    } catch (const std::exception& e) {
        metrics_.failed_requests++;
        std::cerr << "[Client] Error listing files: " << e.what() << "\n";
        logOperation("list", false);
        return false;
    }
}

bool Client::getFile(const std::string& filename, const std::string& saveDir) {
    if (!isConnected()) {
        std::cerr << "[Client] Not connected to server\n";
        return false;
    }

    if (verbose_) {
        std::cout << "[Client] Downloading file: " << filename << "\n";
        std::cout << "[Client] Save directory: " << saveDir << "\n";
    }

    try {
        metrics_.total_requests++;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        protocol_->request_get(filename, saveDir);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        double duration_ms = duration_us.count() / 1000.0;
        
        // Update RTT with exponential moving average
        if (metrics_.rtt_ms == 0.0) {
            metrics_.rtt_ms = duration_ms;
        } else {
            metrics_.rtt_ms = (metrics_.rtt_ms * 0.7) + (duration_ms * 0.3);
        }
        metrics_.transfer_latency_ms = duration_ms;
        
        // Calculate throughput (simplified - would need actual file size)
        // metrics_.throughput_kbps = (fileSize * 8.0) / (duration.count() / 1000.0) / 1024.0;
        
        updateMetrics();
        logOperation("get:" + filename, true);
        return true;
    } catch (const std::exception& e) {
        metrics_.failed_requests++;
        std::cerr << "[Client] Error downloading file: " << e.what() << "\n";
        logOperation("get:" + filename, false);
        return false;
    }
}

bool Client::putFile(const std::string& filepath) {
    if (!isConnected()) {
        std::cerr << "[Client] Not connected to server\n";
        return false;
    }

    if (verbose_) {
        std::cout << "[Client] Uploading file: " << filepath << "\n";
    }

    try {
        metrics_.total_requests++;
        auto startTime = std::chrono::high_resolution_clock::now();
        
        protocol_->request_put(filepath);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        double duration_ms = duration_us.count() / 1000.0;
        
        // Update RTT with exponential moving average
        if (metrics_.rtt_ms == 0.0) {
            metrics_.rtt_ms = duration_ms;
        } else {
            metrics_.rtt_ms = (metrics_.rtt_ms * 0.7) + (duration_ms * 0.3);
        }
        metrics_.transfer_latency_ms = duration_ms;
        
        updateMetrics();
        logOperation("put:" + filepath, true);
        return true;
    } catch (const std::exception& e) {
        metrics_.failed_requests++;
        std::cerr << "[Client] Error uploading file: " << e.what() << "\n";
        logOperation("put:" + filepath, false);
        return false;
    }
}

// Metrics and Statistics
const ClientMetrics& Client::getMetrics() const {
    return metrics_;
}

void Client::resetMetrics() {
    metrics_ = ClientMetrics{};
    
    if (verbose_) {
        std::cout << "[Client] Metrics reset\n";
    }
}

bool Client::exportMetrics(const std::string& filename) const {
    try {
        metrics_.log_csv(filename);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Client] Error exporting metrics: " << e.what() << "\n";
        return false;
    }
}

void Client::displayMetrics() const {
    // Calculate packet loss rate
    double packet_loss = 0.0;
    if (metrics_.total_requests > 0) {
        packet_loss = (static_cast<double>(metrics_.failed_requests) / 
                      static_cast<double>(metrics_.total_requests)) * 100.0;
    }
    
    std::cout << "\n=== Client Metrics ===\n";
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "RTT:                 " << metrics_.rtt_ms << " ms\n";
    std::cout << "Throughput:          " << metrics_.throughput_kbps << " kbps\n";
    std::cout << "Packet Loss Rate:    " << packet_loss << " %\n";
    std::cout << "Transfer Latency:    " << metrics_.transfer_latency_ms << " ms\n";
    std::cout << "Total Requests:      " << metrics_.total_requests << "\n";
    std::cout << "Failed Requests:     " << metrics_.failed_requests << "\n";
    std::cout << "=====================\n\n";
}

// Configuration
void Client::setTimeout(int seconds) {
    timeout_ = seconds;
    
    if (verbose_) {
        std::cout << "[Client] Timeout set to " << timeout_ << " seconds\n";
    }
}

void Client::setVerbose(bool enable) {
    verbose_ = enable;
    
    if (verbose_) {
        std::cout << "[Client] Verbose mode enabled\n";
    }
}

// Private Helper Methods
void Client::updateMetrics() {
    // This method can be extended to update more sophisticated metrics
    // For now, it's a placeholder for future enhancements
    
    if (verbose_) {
        std::cout << "[Client] Metrics updated\n";
    }
}

void Client::logOperation(const std::string& operation, bool success) {
    if (verbose_) {
        std::cout << "[Client] Operation '" << operation << "': " 
                  << (success ? "SUCCESS" : "FAILED") << "\n";
    }
}
