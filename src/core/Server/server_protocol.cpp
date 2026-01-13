#include "server_protocol.h"
#include "server_socket.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <chrono>

ServerProtocol::ServerProtocol() 
    : sharedDirectory_(std::make_shared<std::string>("./shared")),
      metrics_(nullptr) {
}

void ServerProtocol::setSharedDirectory(const std::string& directory) {
    *sharedDirectory_ = directory;
}

void ServerProtocol::setSharedDirectoryPtr(std::shared_ptr<std::string> directoryPtr) {
    sharedDirectory_ = directoryPtr;
}

void ServerProtocol::setMetrics(ServerMetrics* metrics) {
    metrics_ = metrics;
}

std::string ServerProtocol::getSharedDirectory() const {
    return *sharedDirectory_;
}

bool ServerProtocol::processRequest(int clientFd) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Read command from client
    uint8_t cmd = 0;
    ssize_t received = ServerSocket::receiveData(clientFd, &cmd, sizeof(cmd));
    
    if (received == 0) {
        // Clean disconnect
        std::cout << "[Protocol] Client disconnected cleanly\n";
        return false;
    }
    
    if (received < 0) {
        std::cerr << "[Protocol] Failed to receive command\n";
        return false;
    }

    bool result = false;
    // Process command
    switch (cmd) {
        case CMD_LIST:
            result = handleListCommand(clientFd);
            break;
        case CMD_GET:
            result = handleGetCommand(clientFd);
            break;
        case CMD_PUT:
            result = handlePutCommand(clientFd);
            break;
        case CMD_PING:
            result = handlePingCommand(clientFd);
            break;
        default:
            std::cerr << "[Protocol] Unknown command: " << (int)cmd << "\n";
            return false;
    }
    
    // Calculate and update latency
    if (metrics_ && result) {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        metrics_->updateLatency(duration.count());
    }
    
    return result;
}

bool ServerProtocol::handleListCommand(int clientFd) {
    std::cout << "[Protocol] Processing LIST command\n";

    auto files = listFiles();
    uint32_t fileCount = files.size();

    // Send file count
    if (ServerSocket::sendData(clientFd, reinterpret_cast<uint8_t*>(&fileCount), sizeof(fileCount)) < 0) {
        std::cerr << "[Protocol] Failed to send file count\n";
        return false;
    }

    // Send each filename
    for (const auto& filename : files) {
        char filenameBuf[256] = {0};
        std::strncpy(filenameBuf, filename.c_str(), sizeof(filenameBuf) - 1);
        
        if (ServerSocket::sendData(clientFd, reinterpret_cast<uint8_t*>(filenameBuf), sizeof(filenameBuf)) < 0) {
            std::cerr << "[Protocol] Failed to send filename\n";
            return false;
        }
    }

    std::cout << "[Protocol] Sent " << fileCount << " files\n";
    return true;
}

bool ServerProtocol::handlePingCommand(int clientFd) {
    std::cout << "[Protocol] Processing PING command\n";
    // Respond immediately with PONG (echo back CMD_PING)
    uint8_t response = CMD_PING;
    std::cout << "[Protocol] Sending PONG: " << (int)response << "\n";
    if (ServerSocket::sendData(clientFd, &response, sizeof(response)) < 0) {
        std::cerr << "[Protocol] Failed to send PONG\n";
        return false;
    }
    std::cout << "[Protocol] PONG sent successfully\n";
    return true;
}

bool ServerProtocol::handleGetCommand(int clientFd) {
    std::cout << "[Protocol] Processing GET command\n";

    // Receive filename
    char filenameBuf[256] = {0};
    if (ServerSocket::receiveData(clientFd, reinterpret_cast<uint8_t*>(filenameBuf), sizeof(filenameBuf)) <= 0) {
        std::cerr << "[Protocol] Failed to receive filename\n";
        return false;
    }

    // Extract filename (stop at first null character, preserving spaces)
    std::string filename(filenameBuf);
    // Find actual end of string (first null character)
    size_t actualLen = 0;
    for (size_t i = 0; i < sizeof(filenameBuf); ++i) {
        if (filenameBuf[i] == '\0') {
            actualLen = i;
            break;
        }
    }
    if (actualLen > 0) {
        filename = std::string(filenameBuf, actualLen);
    }
    
    std::cout << "[Protocol] Client requested file: '" << filename << "' (length: " << filename.length() << ")\n";

    return sendFile(clientFd, filename);
}

bool ServerProtocol::handlePutCommand(int clientFd) {
    std::cout << "[Protocol] Processing PUT command\n";

    // Receive filename
    char filenameBuf[256] = {0};
    if (ServerSocket::receiveData(clientFd, reinterpret_cast<uint8_t*>(filenameBuf), sizeof(filenameBuf)) <= 0) {
        std::cerr << "[Protocol] Failed to receive filename\n";
        return false;
    }

    // Receive file size
    uint64_t fileSize = 0;
    if (ServerSocket::receiveData(clientFd, reinterpret_cast<uint8_t*>(&fileSize), sizeof(fileSize)) <= 0) {
        std::cerr << "[Protocol] Failed to receive file size\n";
        return false;
    }

    // Extract filename (stop at first null character, preserving spaces)
    std::string filename(filenameBuf);
    // Find actual end of string (first null character)
    size_t actualLen = 0;
    for (size_t i = 0; i < sizeof(filenameBuf); ++i) {
        if (filenameBuf[i] == '\0') {
            actualLen = i;
            break;
        }
    }
    if (actualLen > 0) {
        filename = std::string(filenameBuf, actualLen);
    }
    
    std::cout << "[Protocol] Receiving file: '" << filename << "' (" << fileSize << " bytes)\n";

    return receiveFile(clientFd, filename, fileSize);
}

std::vector<std::string> ServerProtocol::listFiles() {
    std::vector<std::string> files;

    std::string currentDir = *sharedDirectory_;
    std::cout << "[Protocol] Listing files in directory: " << currentDir << "\n";

    DIR* dir = opendir(currentDir.c_str());
    if (!dir) {
        std::cerr << "[Protocol] Failed to open directory: " << currentDir 
                  << " (errno: " << errno << " - " << strerror(errno) << ")\n";
        return files;
    }

    struct dirent* entry;
    int entryCount = 0;
    while ((entry = readdir(dir)) != nullptr) {
        entryCount++;
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Check if it's a regular file
        std::string filepath = currentDir + "/" + entry->d_name;
        struct stat fileStat;
        if (stat(filepath.c_str(), &fileStat) == 0) {
            if (S_ISREG(fileStat.st_mode)) {
                std::cout << "[Protocol] Found file: " << entry->d_name << "\n";
                files.push_back(entry->d_name);
            } else {
                std::cout << "[Protocol] Skipping non-regular file: " << entry->d_name << "\n";
            }
        } else {
            std::cerr << "[Protocol] Failed to stat: " << filepath << " (errno: " << errno << ")\n";
        }
    }

    std::cout << "[Protocol] Total directory entries: " << entryCount 
              << ", Regular files found: " << files.size() << "\n";

    closedir(dir);
    return files;
}

bool ServerProtocol::sendFile(int clientFd, const std::string& filename) {
    std::string filepath = *sharedDirectory_ + "/" + filename;

    // Check if file exists
    struct stat fileStat;
    uint64_t fileSize = 0;
    
    if (stat(filepath.c_str(), &fileStat) != 0) {
        std::cerr << "[Protocol] File not found: " << filepath << "\n";
        // Send zero file size to indicate file not found
        ServerSocket::sendData(clientFd, reinterpret_cast<uint8_t*>(&fileSize), sizeof(fileSize));
        return true; // Continue session, client will handle gracefully
    }

    fileSize = fileStat.st_size;

    // Send file size
    if (ServerSocket::sendData(clientFd, reinterpret_cast<uint8_t*>(&fileSize), sizeof(fileSize)) < 0) {
        std::cerr << "[Protocol] Failed to send file size\n";
        return false;
    }

    // Open file
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "[Protocol] Failed to open file: " << filepath << "\n";
        return false;
    }

    // Send file data
    const size_t BUFFER_SIZE = 64*1024;
    char buffer[BUFFER_SIZE];
    uint64_t totalSent = 0;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastUpdateTime = startTime;

    while (file && totalSent < fileSize) {
        file.read(buffer, BUFFER_SIZE);
        std::streamsize bytesRead = file.gcount();

        if (ServerSocket::sendData(clientFd, reinterpret_cast<uint8_t*>(buffer), bytesRead) < 0) {
            std::cerr << "[Protocol] Failed to send file data\n";
            file.close();
            return false;
        }

        totalSent += bytesRead;
        
        // Update metrics in real-time every 100ms
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime);
        
        if (elapsed.count() >= 100 || totalSent == fileSize) {
            auto totalElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
            
            if (metrics_ && totalElapsed.count() > 0) {
                metrics_->updateThroughput(totalSent, totalElapsed.count());
            }
            
            lastUpdateTime = currentTime;
        }
    }

    file.close();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Update metrics
    if (metrics_) {
        metrics_->addBytesSent(totalSent);
        metrics_->filesDownloaded++;
        
        // Calculate and update throughput
        if (duration.count() > 0) {
            double throughput_kbps = (totalSent * 8.0) / duration.count();
            metrics_->updateThroughput(totalSent, duration.count());
        }
    }
    
    std::cout << "[Protocol] File sent successfully: " << filename << " (" << totalSent << " bytes)\n";
    return true;
}

bool ServerProtocol::receiveFile(int clientFd, const std::string& filename, uint64_t fileSize) {
    std::string filepath = *sharedDirectory_ + "/" + filename;

    // Create output file
    std::ofstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "[Protocol] Failed to create file: " << filepath << "\n";
        return false;
    }

    // Receive file data
    const size_t BUFFER_SIZE = 64*1024;
    uint8_t buffer[BUFFER_SIZE];
    uint64_t totalReceived = 0;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    auto lastUpdateTime = startTime;

    while (totalReceived < fileSize) {
        size_t toReceive = std::min(BUFFER_SIZE, static_cast<size_t>(fileSize - totalReceived));
        ssize_t received = ServerSocket::receiveData(clientFd, buffer, toReceive);
        
        if (received <= 0) {
            std::cerr << "[Protocol] Failed to receive file data\n";
            file.close();
            // Delete partial file on error
            unlink(filepath.c_str());
            return false;
        }

        file.write(reinterpret_cast<char*>(buffer), received);
        totalReceived += received;
        
        // Update metrics in real-time every 100ms
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime);
        
        if (elapsed.count() >= 100 || totalReceived == fileSize) {
            auto totalElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);
            
            if (metrics_ && totalElapsed.count() > 0) {
                metrics_->updateThroughput(totalReceived, totalElapsed.count());
            }
            
            lastUpdateTime = currentTime;
        }
    }

    file.close();
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    // Update metrics
    if (metrics_) {
        metrics_->addBytesReceived(totalReceived);
        metrics_->filesUploaded++;
        
        // Calculate and update throughput
        if (duration.count() > 0) {
            double throughput_kbps = (totalReceived * 8.0) / duration.count();
            metrics_->updateThroughput(totalReceived, duration.count());
        }
    }
    
    std::cout << "[Protocol] File received successfully: " << filename << " (" << totalReceived << " bytes)\n";
    return true;
}