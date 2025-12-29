#include "client_session.h"
#include "server_socket.h"
#include "server_protocol.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

ClientSession::ClientSession(int clientFd, const std::string& clientAddr, std::shared_ptr<std::string> sharedDir, ServerMetrics* metrics)
    : clientFd_(clientFd),
      clientAddr_(clientAddr),
      sharedDir_(sharedDir),
      metrics_(metrics),
      active_(false),
      bytesTransferred_(0) {
    startTime_ = std::chrono::system_clock::now();
}

ClientSession::~ClientSession() {
    // Set flag to stop processing
    active_ = false;
    
    // Detach thread if it's still running - let it clean up itself
    if (thread_ && thread_->joinable()) {
        thread_->detach();
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
    // Just signal the thread to stop
    active_ = false;
    
    // Don't join or detach - let destructor handle it
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
        protocol.setSharedDirectoryPtr(sharedDir_);
        protocol.setMetrics(metrics_);

        // Process client requests
        while (active_) {
            try {
                bool continueSession = protocol.processRequest(clientFd_);
                
                if (!continueSession) {
                    std::cout << "[Session] Session ended normally\n";
                    break;
                }
            } catch (const std::exception& e) {
                std::cerr << "[Session] Exception during request processing: " << e.what() << "\n";
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "[Session] Exception in session setup: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "[Session] Unknown exception in session\n";
    }

    // Cleanup must happen before marking inactive
    cleanup();
    
    std::cout << "[Session] Client disconnected: " << clientAddr_ << "\n";
    
    // Mark inactive as last step - this signals to cleanup thread that we're done
    active_ = false;
}

void ClientSession::cleanup() {
    if (clientFd_ >= 0) {
        close(clientFd_);
        clientFd_ = -1;
    }
}