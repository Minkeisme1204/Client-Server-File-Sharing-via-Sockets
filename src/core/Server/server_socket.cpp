#include "server_socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <errno.h>
#include <signal.h>

ServerSocket::ServerSocket() 
    : socketFd_(-1),
      port_(0),
      listening_(false) {
}

ServerSocket::~ServerSocket() {
    close();
}

bool ServerSocket::bind(uint16_t port, int backlog) {
    // Create socket
    socketFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd_ < 0) {
        std::cerr << "[ServerSocket] Failed to create socket: " << strerror(errno) << "\n";
        return false;
    }

    // Set socket options - allow address reuse
    int opt = 1;
    if (setsockopt(socketFd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "[ServerSocket] Failed to set SO_REUSEADDR: " << strerror(errno) << "\n";
        ::close(socketFd_);
        socketFd_ = -1;
        return false;
    }

    // Setup server address
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // Bind socket to address
    if (::bind(socketFd_, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "[ServerSocket] Failed to bind to port " << port << ": " << strerror(errno) << "\n";
        ::close(socketFd_);
        socketFd_ = -1;
        return false;
    }

    // Listen for connections
    if (listen(socketFd_, backlog) < 0) {
        std::cerr << "[ServerSocket] Failed to listen: " << strerror(errno) << "\n";
        ::close(socketFd_);
        socketFd_ = -1;
        return false;
    }

    port_ = port;
    listening_ = true;

    std::cout << "[ServerSocket] Server listening on port " << port_ << "\n";
    return true;
}

int ServerSocket::acceptConnection(std::string& clientAddr) {
    if (!listening_) {
        return -1;
    }

    struct sockaddr_in clientAddrStruct;
    socklen_t clientAddrLen = sizeof(clientAddrStruct);

    int clientFd = accept(socketFd_, (struct sockaddr*)&clientAddrStruct, &clientAddrLen);
    
    if (clientFd < 0) {
        if (errno != EINTR) { // Ignore interrupt errors
            std::cerr << "[ServerSocket] Accept failed: " << strerror(errno) << "\n";
        }
        return -1;
    }

    // Get client IP address
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddrStruct.sin_addr, ipStr, sizeof(ipStr));
    clientAddr = std::string(ipStr) + ":" + std::to_string(ntohs(clientAddrStruct.sin_port));

    return clientFd;
}

void ServerSocket::close() {
    if (socketFd_ >= 0) {
        ::close(socketFd_);
        socketFd_ = -1;
        listening_ = false;
    }
}

bool ServerSocket::isListening() const {
    return listening_;
}

int ServerSocket::getSocketFd() const {
    return socketFd_;
}

ssize_t ServerSocket::sendData(int fd, const uint8_t* data, size_t size) {
    if (fd < 0 || !data) {
        return -1;
    }

    size_t totalSent = 0;
    while (totalSent < size) {
        ssize_t sent = send(fd, data + totalSent, size - totalSent, MSG_NOSIGNAL);
        if (sent < 0) {
            if (errno == EINTR) {
                // Interrupted, try again
                continue;
            }
            if (errno == EPIPE || errno == ECONNRESET) {
                // Broken pipe or connection reset - client disconnected
                std::cerr << "[ServerSocket] Client disconnected during send\n";
                return -1;
            }
            std::cerr << "[ServerSocket] Send failed: " << strerror(errno) << "\n";
            return -1;
        }
        if (sent == 0) {
            return totalSent;
        }
        totalSent += sent;
    }

    return totalSent;
}

ssize_t ServerSocket::receiveData(int fd, uint8_t* buffer, size_t size) {
    if (fd < 0 || !buffer) {
        return -1;
    }

    size_t totalReceived = 0;
    while (totalReceived < size) {
        ssize_t received = recv(fd, buffer + totalReceived, size - totalReceived, 0);
        if (received < 0) {
            if (errno == EINTR) {
                // Interrupted, try again
                continue;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Timeout - this is expected with SO_RCVTIMEO, not an error
                // Return what we have so far
                return totalReceived > 0 ? totalReceived : 0;
            }
            if (errno == ECONNRESET || errno == EPIPE) {
                // Connection reset by peer - clean disconnect
                return 0;
            }
            std::cerr << "[ServerSocket] Receive failed: " << strerror(errno) << "\n";
            return -1;
        }
        if (received == 0) {
            // Clean disconnect
            if (totalReceived == 0) {
                return 0; // Client closed connection before sending anything
            }
            return totalReceived; // Partial data received
        }
        totalReceived += received;
    }

    return totalReceived;
}