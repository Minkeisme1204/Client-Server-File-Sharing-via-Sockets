#include "client_protocol.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <chrono>

// Protocol command codes
#define CMD_LIST 0x01
#define CMD_GET  0x02
#define CMD_PUT  0x03

ClientProtocol::ClientProtocol(ClientSocket &socket) 
    : socket_(socket), metrics_(nullptr) {
}

void ClientProtocol::setMetrics(ClientMetrics* metrics) {
    metrics_ = metrics;
}

void ClientProtocol::request_list() {
    if (!socket_.isConnected()) {
        std::cerr << "[Protocol] Not connected to server\n";
        return;
    }

    // Send LIST command
    uint8_t cmd = CMD_LIST;
    std::cout << "[ClientProtocol] Sending LIST command...\n";
    if (socket_.sendData(&cmd, sizeof(cmd)) < 0) {
        std::cerr << "[Protocol] Failed to send LIST command\n";
        return;
    }
    std::cout << "[ClientProtocol] LIST command sent successfully\n";

    // Receive number of files
    uint32_t fileCount = 0;
    std::cout << "[ClientProtocol] Waiting for file count...\n";
    ssize_t received = socket_.receiveData(reinterpret_cast<uint8_t*>(&fileCount), sizeof(fileCount));
    if (received != sizeof(fileCount)) {
        std::cerr << "[Protocol] Failed to receive file count (received: " << received << " bytes)\n";
        return;
    }
    std::cout << "[ClientProtocol] Received file count: " << fileCount << "\n";

    std::cout << "\n┌─────────────── FILES ON SERVER ───────────────┐\n";
    if (fileCount == 0) {
        std::cout << "│ No files available                            │\n";
    } else {
        std::cout << "│ Total files: " << fileCount << std::string(33 - std::to_string(fileCount).length(), ' ') << "│\n";
        std::cout << "├───────────────────────────────────────────────┤\n";
    }
    
    // Receive and display each filename
    for (uint32_t i = 0; i < fileCount; i++) {
        char filename[256] = {0};
        std::cout << "[ClientProtocol] Receiving filename " << (i+1) << "...\n";
        received = socket_.receiveData(reinterpret_cast<uint8_t*>(filename), sizeof(filename));
        if (received != sizeof(filename)) {
            std::cerr << "[Protocol] Failed to receive filename (received: " << received << " bytes)\n";
            break;
        }
        std::cout << "│ " << std::setw(2) << (i + 1) << ". " << std::left << std::setw(42) << filename << "│\n";
    }
    
    if (fileCount > 0) {
        std::cout << "└───────────────────────────────────────────────┘\n";
    } else {
        std::cout << "└───────────────────────────────────────────────┘\n";
    }
}

std::vector<std::string> ClientProtocol::requestFileList() {
    std::vector<std::string> fileList;
    
    if (!socket_.isConnected()) {
        std::cerr << "[Protocol] Not connected to server\n";
        return fileList;
    }

    // Send LIST command
    uint8_t cmd = CMD_LIST;
    if (socket_.sendData(&cmd, sizeof(cmd)) < 0) {
        std::cerr << "[Protocol] Failed to send LIST command\n";
        return fileList;
    }

    // Receive number of files
    uint32_t fileCount = 0;
    ssize_t received = socket_.receiveData(reinterpret_cast<uint8_t*>(&fileCount), sizeof(fileCount));
    if (received != sizeof(fileCount)) {
        std::cerr << "[Protocol] Failed to receive file count\n";
        return fileList;
    }

    // Receive each filename
    for (uint32_t i = 0; i < fileCount; i++) {
        char filename[256] = {0};
        received = socket_.receiveData(reinterpret_cast<uint8_t*>(filename), sizeof(filename));
        if (received != sizeof(filename)) {
            std::cerr << "[Protocol] Failed to receive filename\n";
            break;
        }
        // Add non-empty filenames to list
        if (filename[0] != '\0') {
            fileList.push_back(std::string(filename));
        }
    }

    return fileList;
}

bool ClientProtocol::request_get(const std::string &filename, const std::string &save_dir) {
    if (!socket_.isConnected()) {
        std::cerr << "[Protocol] Not connected to server\n";
        return false;
    }

    // Send GET command
    uint8_t cmd = CMD_GET;
    if (socket_.sendData(&cmd, sizeof(cmd)) < 0) {
        std::cerr << "[Protocol] Failed to send GET command\n";
        return false;
    }

    // Send filename
    char filenameBuf[256] = {0};
    std::strncpy(filenameBuf, filename.c_str(), sizeof(filenameBuf) - 1);
    if (socket_.sendData(reinterpret_cast<uint8_t*>(filenameBuf), sizeof(filenameBuf)) < 0) {
        std::cerr << "[Protocol] Failed to send filename\n";
        return false;
    }

    // Receive file size
    uint64_t fileSize = 0;
    if (socket_.receiveData(reinterpret_cast<uint8_t*>(&fileSize), sizeof(fileSize)) < 0) {
        std::cerr << "[Protocol] Failed to receive file size\n";
        return false;
    }

    if (fileSize == 0) {
        std::cerr << "[Protocol] File not found on server\n";
        return false;
    }

    // Create output file path
    std::string outputPath = save_dir.empty() ? filename : save_dir + "/" + filename;
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "[Protocol] Failed to create file: " << outputPath << "\n";
        return false;
    }

    std::cout << "[Protocol] Downloading " << filename << " (" << fileSize << " bytes)\n";

    // Receive file data
    const size_t BUFFER_SIZE = 1024*64;
    uint8_t buffer[BUFFER_SIZE];
    uint64_t totalReceived = 0;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastUpdateTime = startTime;

    while (totalReceived < fileSize) {
        size_t toReceive = std::min(BUFFER_SIZE, static_cast<size_t>(fileSize - totalReceived));
        ssize_t received = socket_.receiveData(buffer, toReceive);
        
        if (received <= 0) {
            std::cerr << "[Protocol] Failed to receive file data\n";
            outFile.close();
            return false;
        }

        outFile.write(reinterpret_cast<char*>(buffer), received);
        totalReceived += received;

        // Update metrics in real-time every 100ms
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime);
        
        if (elapsed.count() >= 100 || totalReceived == fileSize) {
            auto totalElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
            
            if (metrics_ && totalElapsed.count() > 0) {
                // Calculate current throughput in kbps
                double currentThroughput = (totalReceived * 8.0) / totalElapsed.count();
                metrics_->throughput_kbps = currentThroughput;
                metrics_->transfer_latency_ms = totalElapsed.count();
                // Note: total_bytes_received is updated in final update to avoid double counting
            }
            
            lastUpdateTime = currentTime;
        }

        // Progress indicator
        if (totalReceived % (1024 * 1024) == 0 || totalReceived == fileSize) {
            std::cout << "\rProgress: " << (totalReceived * 100 / fileSize) << "% " << std::flush;
        }
    }

    std::cout << "\n[Protocol] Download completed: " << outputPath << "\n";
    outFile.close();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    uint64_t duration_ms = duration.count() / 1000;  // Convert to milliseconds
    
    // For very fast transfers, ensure at least 1ms is recorded
    if (duration_ms == 0 && duration.count() > 0) {
        duration_ms = 1;
    }
    
    std::cout << "[Protocol DEBUG] Transfer duration: " << duration.count() << " us (" << duration_ms << " ms)" << std::endl;
    std::cout << "[Protocol DEBUG] metrics_ pointer: " << (metrics_ ? "valid" : "NULL") << std::endl;
    
    // Update metrics
    if (metrics_) {
        metrics_->transfer_latency_ms = duration_ms;
        metrics_->total_bytes_received += fileSize;  // Downloaded bytes
        metrics_->total_transfer_time_ms += duration_ms;
        
        std::cout << "[Protocol DEBUG] Updated total_transfer_time_ms: " 
                  << metrics_->total_transfer_time_ms.load() << " ms" << std::endl;
        
        // Calculate average throughput across all transfers
        uint64_t total_time = metrics_->total_transfer_time_ms.load();
        if (total_time > 0) {
            uint64_t total_bytes = metrics_->total_bytes_sent.load() + metrics_->total_bytes_received.load();
            metrics_->throughput_kbps = (total_bytes * 8.0) / total_time;
        }
    } else {
        std::cout << "[Protocol DEBUG] metrics_ is NULL - cannot update!" << std::endl;
    }
    return true;
}

bool ClientProtocol::request_put(const std::string &filepath) {
    if (!socket_.isConnected()) {
        std::cerr << "[Protocol] Not connected to server\n";
        return false;
    }

    // Check if file exists
    struct stat fileStat;
    if (stat(filepath.c_str(), &fileStat) != 0) {
        std::cerr << "[Protocol] File not found: " << filepath << "\n";
        return false;
    }

    uint64_t fileSize = fileStat.st_size;

    // Extract filename from path
    size_t pos = filepath.find_last_of("/\\");
    std::string filename = (pos != std::string::npos) ? filepath.substr(pos + 1) : filepath;

    // Open file
    std::ifstream inFile(filepath, std::ios::binary);
    if (!inFile) {
        std::cerr << "[Protocol] Failed to open file: " << filepath << "\n";
        return false;
    }

    // Send PUT command
    uint8_t cmd = CMD_PUT;
    if (socket_.sendData(&cmd, sizeof(cmd)) < 0) {
        std::cerr << "[Protocol] Failed to send PUT command\n";
        return false;
    }

    // Send filename
    char filenameBuf[256] = {0};
    std::strncpy(filenameBuf, filename.c_str(), sizeof(filenameBuf) - 1);
    if (socket_.sendData(reinterpret_cast<uint8_t*>(filenameBuf), sizeof(filenameBuf)) < 0) {
        std::cerr << "[Protocol] Failed to send filename\n";
        return false;
    }

    // Send file size
    if (socket_.sendData(reinterpret_cast<uint8_t*>(&fileSize), sizeof(fileSize)) < 0) {
        std::cerr << "[Protocol] Failed to send file size\n";
        return false;
    }

    std::cout << "[Protocol] Uploading " << filename << " (" << fileSize << " bytes)\n";

    // Send file data
    const size_t BUFFER_SIZE = 64*1024;
    char buffer[BUFFER_SIZE];
    uint64_t totalSent = 0;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastUpdateTime = startTime;

    while (inFile && totalSent < fileSize) {
        inFile.read(buffer, BUFFER_SIZE);
        std::streamsize bytesRead = inFile.gcount();

        if (socket_.sendData(reinterpret_cast<uint8_t*>(buffer), bytesRead) < 0) {
            std::cerr << "[Protocol] Failed to send file data\n";
            inFile.close();
            return false;
        }

        totalSent += bytesRead;

        // Update metrics in real-time every 100ms
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime);
        
        if (elapsed.count() >= 100 || totalSent == fileSize) {
            auto totalElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
            
            if (metrics_ && totalElapsed.count() > 0) {
                // Calculate current throughput in kbps
                double currentThroughput = (totalSent * 8.0) / totalElapsed.count();
                metrics_->throughput_kbps = currentThroughput;
                metrics_->transfer_latency_ms = totalElapsed.count();
                // Note: total_bytes_sent is updated in final update to avoid double counting
            }
            
            lastUpdateTime = currentTime;
        }

        // Progress indicator
        if (totalSent % (1024 * 1024) == 0 || totalSent == fileSize) {
            std::cout << "\rProgress: " << (totalSent * 100 / fileSize) << "% " << std::flush;
        }
    }

    std::cout << "\n[Protocol] Upload completed\n";
    inFile.close();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    uint64_t duration_ms = duration.count() / 1000;  // Convert to milliseconds
    
    // For very fast transfers, ensure at least 1ms is recorded
    if (duration_ms == 0 && duration.count() > 0) {
        duration_ms = 1;
    }
    
    // Update metrics
    if (metrics_) {
        metrics_->transfer_latency_ms = duration_ms;
        metrics_->total_bytes_sent += fileSize;  // Uploaded bytes
        metrics_->total_transfer_time_ms += duration_ms;
        
        // Calculate average throughput across all transfers
        uint64_t total_time = metrics_->total_transfer_time_ms.load();
        if (total_time > 0) {
            uint64_t total_bytes = metrics_->total_bytes_sent.load() + metrics_->total_bytes_received.load();
            metrics_->throughput_kbps = (total_bytes * 8.0) / total_time;
        }
    }
    return true;
}
