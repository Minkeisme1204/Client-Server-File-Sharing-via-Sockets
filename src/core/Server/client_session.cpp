#include "client_session.h"
#include "server_socket.h"
#include "server_protocol.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

ClientSession::ClientSession(int clientFd, const std::string& clientAddr, const std::string& sharedDir)
    : clientFd_(clientFd),
      clientAddr_(clientAddr),
      sharedDir_(sharedDir),
      active_(false),
      bytesTransferred_(0) {
    startTime_ = std::chrono::system_clock::now();
}

ClientSession::~ClientSession() {
    stop();
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
    
    // Wait for thread to finish
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

    try {
        // Create protocol handler for this session
        ServerProtocol protocol;
        protocol.setSharedDirectory(sharedDir_);

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