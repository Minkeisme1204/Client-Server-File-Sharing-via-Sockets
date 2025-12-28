#include "client_protocol.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <sys/socket.h>
#include <stdexcept>
#include <vector>

// Protocol command codes
#define CMD_LIST 0x01
#define CMD_GET  0x02
#define CMD_PUT  0x03

ClientProtocol::ClientProtocol(ClientSocket &socket) : socket_(socket) {
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

uint64_t ClientProtocol::request_get(const std::string &filename, const std::string &save_dir) {
    if (!socket_.isConnected()) {
        std::cerr << "[Protocol] Not connected to server\n";
        return 0;
    }

    // Send GET command
    uint8_t cmd = CMD_GET;
    if (socket_.sendData(&cmd, sizeof(cmd)) < 0) {
        std::cerr << "[Protocol] Failed to send GET command\n";
        return 0;
    }

    // Send filename
    char filenameBuf[256] = {0};
    std::strncpy(filenameBuf, filename.c_str(), sizeof(filenameBuf) - 1);
    if (socket_.sendData(reinterpret_cast<uint8_t*>(filenameBuf), sizeof(filenameBuf)) < 0) {
        std::cerr << "[Protocol] Failed to send filename\n";
        return 0;
    }

    // Receive file size
    uint64_t fileSize = 0;
    if (socket_.receiveData(reinterpret_cast<uint8_t*>(&fileSize), sizeof(fileSize)) < 0) {
        std::cerr << "[Protocol] Failed to receive file size\n";
        return 0;
    }

    if (fileSize == 0) {
        std::cerr << "[Protocol] File not found on server\n";
        return 0;
    }

    // Create output file path
    std::string outputPath = save_dir.empty() ? filename : save_dir + "/" + filename;
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "[Protocol] Failed to create file: " << outputPath << "\n";
        return 0;
    }

    std::cout << "[Protocol] Downloading " << filename << " (" << fileSize << " bytes)\n";

    // Receive file data
    const size_t BUFFER_SIZE = 8192;
    uint8_t buffer[BUFFER_SIZE];
    uint64_t totalReceived = 0;

    while (totalReceived < fileSize) {
        size_t toReceive = std::min(BUFFER_SIZE, static_cast<size_t>(fileSize - totalReceived));
        ssize_t received = socket_.receiveData(buffer, toReceive);
        
        if (received <= 0) {
            std::cerr << "[Protocol] Failed to receive file data\n";
            outFile.close();
            return 0;
        }

        outFile.write(reinterpret_cast<char*>(buffer), received);
        totalReceived += received;

        // Progress indicator
        if (totalReceived % (1024 * 1024) == 0 || totalReceived == fileSize) {
            std::cout << "\rProgress: " << (totalReceived * 100 / fileSize) << "% " << std::flush;
        }
    }

    std::cout << "\n[Protocol] Download completed: " << outputPath << "\n";
    outFile.close();
    return fileSize;
}

uint64_t ClientProtocol::request_put(const std::string &filepath) {
    if (!socket_.isConnected()) {
        std::cerr << "[Protocol] Not connected to server\n";
        return 0;
    }

    // Check if file exists
    struct stat fileStat;
    if (stat(filepath.c_str(), &fileStat) != 0) {
        std::cerr << "[Protocol] File not found: " << filepath << "\n";
        return 0;
    }

    uint64_t fileSize = fileStat.st_size;

    // Extract filename from path
    size_t pos = filepath.find_last_of("/\\");
    std::string filename = (pos != std::string::npos) ? filepath.substr(pos + 1) : filepath;

    // Open file
    std::ifstream inFile(filepath, std::ios::binary);
    if (!inFile) {
        std::cerr << "[Protocol] Failed to open file: " << filepath << "\n";
        return 0;
    }

    // Send PUT command
    uint8_t cmd = CMD_PUT;
    if (socket_.sendData(&cmd, sizeof(cmd)) < 0) {
        std::cerr << "[Protocol] Failed to send PUT command\n";
        return 0;
    }

    // Send filename
    char filenameBuf[256] = {0};
    std::strncpy(filenameBuf, filename.c_str(), sizeof(filenameBuf) - 1);
    if (socket_.sendData(reinterpret_cast<uint8_t*>(filenameBuf), sizeof(filenameBuf)) < 0) {
        std::cerr << "[Protocol] Failed to send filename\n";
        return 0;
    }

    // Send file size
    if (socket_.sendData(reinterpret_cast<uint8_t*>(&fileSize), sizeof(fileSize)) < 0) {
        std::cerr << "[Protocol] Failed to send file size\n";
        return 0;
    }

    std::cout << "[Protocol] Uploading " << filename << " (" << fileSize << " bytes)\n";

    // Send file data
    const size_t BUFFER_SIZE = 8192;
    char buffer[BUFFER_SIZE];
    uint64_t totalSent = 0;

    while (inFile && totalSent < fileSize) {
        inFile.read(buffer, BUFFER_SIZE);
        std::streamsize bytesRead = inFile.gcount();

        if (socket_.sendData(reinterpret_cast<uint8_t*>(buffer), bytesRead) < 0) {
            std::cerr << "[Protocol] Failed to send file data\n";
            inFile.close();
            return 0;
        }

        totalSent += bytesRead;

        // Progress indicator
        if (totalSent % (1024 * 1024) == 0 || totalSent == fileSize) {
            std::cout << "\rProgress: " << (totalSent * 100 / fileSize) << "% " << std::flush;
        }
    }

    std::cout << "\n[Protocol] Upload completed\n";
    inFile.close();
    return fileSize;
}

void ClientProtocol::sendListCommand(int sockfd) {
    uint8_t cmd = CMD_LIST;
    if (send(sockfd, &cmd, sizeof(cmd), 0) < 0) {
        throw std::runtime_error("Failed to send LIST command");
    }
}

std::vector<std::string> ClientProtocol::receiveFileList(int sockfd) {
    std::vector<std::string> files;
    
    // Receive number of files
    uint32_t fileCount = 0;
    if (recv(sockfd, &fileCount, sizeof(fileCount), 0) != sizeof(fileCount)) {
        throw std::runtime_error("Failed to receive file count");
    }
    
    // Receive each filename
    for (uint32_t i = 0; i < fileCount; i++) {
        char filename[256] = {0};
        if (recv(sockfd, filename, sizeof(filename), 0) != sizeof(filename)) {
            throw std::runtime_error("Failed to receive filename");
        }
        files.push_back(std::string(filename));
    }
    
    return files;
}
