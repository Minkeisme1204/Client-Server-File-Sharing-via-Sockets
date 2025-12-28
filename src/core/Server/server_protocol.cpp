#include "server_protocol.h"
#include "server_socket.h"
#include "server_metrics.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

ServerProtocol::ServerProtocol() 
    : sharedDirectory_("./shared"), metrics_(nullptr) {
}

void ServerProtocol::setSharedDirectory(const std::string& directory) {
    sharedDirectory_ = directory;
}

std::string ServerProtocol::getSharedDirectory() const {
    return sharedDirectory_;
}

void ServerProtocol::setMetrics(ServerMetrics* metrics) {
    metrics_ = metrics;
}

bool ServerProtocol::processRequest(int clientFd) {
    // Read command from client
    uint8_t cmd = 0;
    ssize_t bytesRead = ServerSocket::receiveData(clientFd, &cmd, sizeof(cmd));
    
    if (bytesRead == 0) {
        // Client disconnected cleanly
        return false;
    }
    
    if (bytesRead < 0) {
        // Error occurred
        std::cerr << "[Protocol] Error reading command from client\n";
        return false;
    }

    // Process command
    switch (cmd) {
        case CMD_LIST:
            return handleListCommand(clientFd);
        case CMD_GET:
            return handleGetCommand(clientFd);
        case CMD_PUT:
            return handlePutCommand(clientFd);
        default:
            std::cerr << "[Protocol] Unknown command: " << (int)cmd << "\n";
            return false;
    }
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

bool ServerProtocol::handleGetCommand(int clientFd) {
    std::cout << "[Protocol] Processing GET command\n";

    // Receive filename
    char filenameBuf[256] = {0};
    if (ServerSocket::receiveData(clientFd, reinterpret_cast<uint8_t*>(filenameBuf), sizeof(filenameBuf)) <= 0) {
        std::cerr << "[Protocol] Failed to receive filename\n";
        return false;
    }

    // Clean filename (remove trailing null/whitespace)
    std::string filename(filenameBuf);
    // Remove null characters and trim
    filename = filename.c_str(); // This stops at first null
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

    // Clean filename
    std::string filename(filenameBuf);
    filename = filename.c_str(); // Stop at first null
    std::cout << "[Protocol] Receiving file: '" << filename << "' (" << fileSize << " bytes)\n";

    return receiveFile(clientFd, filename, fileSize);
}

std::vector<std::string> ServerProtocol::listFiles() {
    std::vector<std::string> files;

    DIR* dir = opendir(sharedDirectory_.c_str());
    if (!dir) {
        std::cerr << "[Protocol] Failed to open directory: " << sharedDirectory_ << "\n";
        return files;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Check if it's a regular file
        std::string filepath = sharedDirectory_ + "/" + entry->d_name;
        struct stat fileStat;
        if (stat(filepath.c_str(), &fileStat) == 0 && S_ISREG(fileStat.st_mode)) {
            files.push_back(entry->d_name);
        }
    }

    closedir(dir);
    return files;
}

bool ServerProtocol::sendFile(int clientFd, const std::string& filename) {
    std::string filepath = sharedDirectory_ + "/" + filename;

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
    const size_t BUFFER_SIZE = 8192;
    char buffer[BUFFER_SIZE];
    uint64_t totalSent = 0;

    while (file && totalSent < fileSize) {
        file.read(buffer, BUFFER_SIZE);
        std::streamsize bytesRead = file.gcount();

        if (ServerSocket::sendData(clientFd, reinterpret_cast<uint8_t*>(buffer), bytesRead) < 0) {
            std::cerr << "[Protocol] Failed to send file data\n";
            file.close();
            return false;
        }

        totalSent += bytesRead;
    }

    file.close();
    std::cout << "[Protocol] File sent successfully: " << filename << " (" << totalSent << " bytes)\n";
    
    // Update metrics
    if (metrics_) {
        metrics_->totalBytesSent += totalSent;
        metrics_->filesDownloaded++;
    }
    
    return true;
}

bool ServerProtocol::receiveFile(int clientFd, const std::string& filename, uint64_t fileSize) {
    std::string filepath = sharedDirectory_ + "/" + filename;

    // Create output file
    std::ofstream file(filepath, std::ios::binary);
    if (!file) {
        std::cerr << "[Protocol] Failed to create file: " << filepath << "\n";
        return false;
    }

    // Receive file data
    const size_t BUFFER_SIZE = 8192;
    uint8_t buffer[BUFFER_SIZE];
    uint64_t totalReceived = 0;

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
    }

    file.close();
    std::cout << "[Protocol] File received successfully: " << filename << " (" << totalReceived << " bytes)\n";
    
    // Update metrics
    if (metrics_) {
        metrics_->totalBytesReceived += totalReceived;
        metrics_->filesUploaded++;
    }
    
    return true;
}