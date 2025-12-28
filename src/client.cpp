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
        auto startTime = std::chrono::high_resolution_clock::now();
        
        protocol_->request_list();
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        metrics_.transfer_latency_ms = duration.count();
        
        logOperation("list", true);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Client] Error listing files: " << e.what() << "\n";
        logOperation("list", false);
        return false;
    }
}

std::vector<std::string> Client::getFileList() {
    std::vector<std::string> files;
    
    if (!isConnected()) {
        std::cerr << "[Client] Not connected to server\n";
        return files;
    }

    try {
        // Send LIST command
        protocol_->sendListCommand(socket_->getSocketFd());

        // Receive file list from protocol
        files = protocol_->receiveFileList(socket_->getSocketFd());
        
        if (verbose_) {
            std::cout << "[Client] Received " << files.size() << " files from server\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "[Client] Error getting file list: " << e.what() << "\n";
    }

    return files;
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
        auto startTime = std::chrono::high_resolution_clock::now();
        
        uint64_t fileSize = protocol_->request_get(filename, saveDir);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        metrics_.transfer_latency_ms = duration.count();
        
        // Calculate throughput
        if (duration.count() > 0) {
            metrics_.throughput_kbps = (fileSize * 8.0) / (duration.count() / 1000.0) / 1024.0;
        }
        
        // Update metrics
        metrics_.totalBytesReceived += fileSize;
        metrics_.filesDownloaded++;
        metrics_.totalOperations++;
        
        updateMetrics();
        logOperation("get:" + filename, true);
        return true;
    } catch (const std::exception& e) {
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
        auto startTime = std::chrono::high_resolution_clock::now();
        
        uint64_t fileSize = protocol_->request_put(filepath);
        
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        metrics_.transfer_latency_ms = duration.count();
        
        // Update metrics (fileSize returned from put)
        metrics_.totalBytesSent += fileSize;
        metrics_.filesUploaded++;
        metrics_.totalOperations++;
        
        updateMetrics();
        logOperation("put:" + filepath, true);
        return true;
    } catch (const std::exception& e) {
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
    std::cout << "\n=== Client Metrics ===\n";
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "RTT:                 " << metrics_.rtt_ms << " ms\n";
    std::cout << "Throughput:          " << metrics_.throughput_kbps << " kbps\n";
    std::cout << "Packet Loss Rate:    " << metrics_.packet_loss_rate << " %\n";
    std::cout << "Transfer Latency:    " << metrics_.transfer_latency_ms << " ms\n";
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
