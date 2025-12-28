#include "client_session.h"
#include "server_socket.h"
#include "server_protocol.h"
#include "server_metrics.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>

ClientSession::ClientSession(int clientFd, const std::string& clientAddr, const std::string& sharedDir, ServerMetrics* metrics)
    : clientFd_(clientFd),
      clientAddr_(clientAddr),
      sharedDir_(sharedDir),
      active_(false),
      bytesTransferred_(0),
      metrics_(metrics) {
    startTime_ = std::chrono::system_clock::now();
}

ClientSession::~ClientSession() {
    try {
        stop();
    } catch (...) {
        // Silently handle exceptions in destructor
    }
}

void ClientSession::start() {
    if (active_) {
        return;
    }

    active_ = true;
    thread_ = std::make_unique<std::thread>(&ClientSession::handleSession, this);
}

void ClientSession::stop() {
    if (!active_) {
        return;
    }
    
    active_ = false;
    
    // Close socket to unblock any I/O operations
    cleanup();
    
    // Wait for thread to finish properly
    if (thread_ && thread_->joinable()) {
        thread_->join();
    }
}

bool ClientSession::isActive() const {
    return active_;
}

std::string ClientSession::getClientAddress() const {
    return clientAddr_;
}

std::chrono::system_clock::time_point ClientSession::getStartTime() const {
    return startTime_;
}

size_t ClientSession::getBytesTransferred() const {
    return bytesTransferred_;
}

void ClientSession::handleSession() {
    std::cout << "[Session] Client connected: " << clientAddr_ << " (fd: " << clientFd_ << ")\n";

    // Validate fd before using it
    if (clientFd_ < 0) {
        std::cerr << "[Session] Invalid socket fd: " << clientFd_ << "\n";
        active_ = false;
        return;
    }

    // Set socket timeout to prevent blocking forever
    struct timeval tv;
    tv.tv_sec = 5;  // 5 second timeout
    tv.tv_usec = 0;
    
    if (setsockopt(clientFd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "[Session] Failed to set socket timeout: " << strerror(errno) << "\n";
        // Continue anyway, just won't have timeout
    }

    try {
        // Create protocol handler for this session
        ServerProtocol protocol;
        protocol.setSharedDirectory(sharedDir_);
        protocol.setMetrics(metrics_);

        // Process client requests
        while (active_) {
            bool continueSession = protocol.processRequest(clientFd_);
            
            if (!continueSession) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[Session] Exception in session: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "[Session] Unknown exception in session\n";
    }

    std::cout << "[Session] Client disconnected: " << clientAddr_ << "\n";
    active_ = false;
}

void ClientSession::cleanup() {
    if (clientFd_ >= 0) {
        close(clientFd_);
        clientFd_ = -1;
    }
}