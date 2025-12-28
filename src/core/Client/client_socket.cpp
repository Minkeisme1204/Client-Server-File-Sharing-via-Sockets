#include "client_socket.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

ClientSocket::ClientSocket() : socketFd_(-1) {
}

ClientSocket::~ClientSocket() {
    if(socketFd_ != -1) disconnect();
}

bool ClientSocket::connectToServer(const std::string& ip, uint16_t port) {
    // Create socket
    socketFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd_ < 0) {
        std::cerr << "[Socket] Failed to create socket\n";
        return false;
    }

    // Setup server address
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    // Convert IP address
    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "[Socket] Invalid address: " << ip << "\n";
        close(socketFd_);
        socketFd_ = -1;
        return false;
    }

    // Connect to server
    if (connect(socketFd_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[Socket] Connection failed to " << ip << ":" << port << "\n";
        close(socketFd_);
        socketFd_ = -1;
        return false;
    }

    return true;
}

void ClientSocket::disconnect() {
    if (socketFd_ >= 0) {
        close(socketFd_);
        socketFd_ = -1;
    }
}

ssize_t ClientSocket::sendData(const uint8_t* data, size_t size) {
    if (socketFd_ < 0 || !data) {
        return -1;
    }

    size_t totalSent = 0;
    while (totalSent < size) {
        ssize_t sent = send(socketFd_, data + totalSent, size - totalSent, 0);
        if (sent < 0) {
            std::cerr << "[Socket] Send failed: " << strerror(errno) << "\n";
            return -1;
        }
        if (sent == 0) {
            std::cerr << "[Socket] Connection closed by peer\n";
            return totalSent;
        }
        totalSent += sent;
    }

    return totalSent;
}

ssize_t ClientSocket::receiveData(uint8_t* buffer, size_t size) {
    if (socketFd_ < 0 || !buffer) {
        return -1;
    }

    size_t totalReceived = 0;
    while (totalReceived < size) {
        ssize_t received = recv(socketFd_, buffer + totalReceived, size - totalReceived, 0);
        if (received < 0) {
            std::cerr << "[Socket] Receive failed: " << strerror(errno) << "\n";
            return -1;
        }
        if (received == 0) {
            // Connection closed
            return totalReceived;
        }
        totalReceived += received;
    }

    return totalReceived;
}

bool ClientSocket::isConnected() const {
    return socketFd_ >= 0;
}

int ClientSocket::getSocketFd() const {
    return socketFd_;
}




