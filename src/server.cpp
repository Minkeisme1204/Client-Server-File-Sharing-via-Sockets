#include "server.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <thread>
#include <sys/stat.h>
#include <unistd.h>

Server::Server()
    : socket_(std::make_unique<ServerSocket>()),
      protocol_(std::make_unique<ServerProtocol>()),
      running_(false),
      sharedDirectory_("./shared"),
      port_(0),
      maxConnections_(0),
      timeout_(30),
      verbose_(false) {
}

Server::~Server() {
    stop();
}

bool Server::start(uint16_t port, const std::string& sharedDir) {
    if (running_) {
        std::cerr << "[Server] Server is already running\n";
        return false;
    }

    // Set shared directory
    if (!setSharedDirectory(sharedDir)) {
        return false;
    }

    // Bind and listen
    if (!socket_->bind(port)) {
        return false;
    }

    port_ = port;
    running_ = true;

    if (verbose_) {
        std::cout << "[Server] Server started on port " << port_ << "\n";
        std::cout << "[Server] Shared directory: " << sharedDirectory_ << "\n";
    }

    logEvent("Server started");
    return true;
}

void Server::stop() {
    if (!running_) {
        return;
    }

    if (verbose_) {
        std::cout << "[Server] Stopping server...\n";
    }

    // Set running flag to false first
    running_ = false;

    // Close socket to unblock accept()
    socket_->close();

    // Wait for accept thread to finish (with timeout)
    if (acceptThread_ && acceptThread_->joinable()) {
        acceptThread_->join();
    }

    // Stop all client sessions
    {
        std::lock_guard<std::mutex> lock(sessionsMutex_);
        for (auto& session : sessions_) {
            if (session) {
                session->stop();
            }
        }
        sessions_.clear();
    }

    if (verbose_) {
        std::cout << "[Server] Server stopped\n";
    }

    logEvent("Server stopped");
}

bool Server::isRunning() const {
    return running_;
}

void Server::run() {
    if (!running_) {
        std::cerr << "[Server] Server not started. Call start() first.\n";
        return;
    }

    acceptLoop();
    std::cout << "[Server] accept loop fine\n";
}

bool Server::setSharedDirectory(const std::string& directory) {
    // Check if directory exists
    struct stat info;
    if (stat(directory.c_str(), &info) != 0) {
        // Directory doesn't exist, try to create it
        if (!createSharedDirectory(directory)) {
            std::cerr << "[Server] Failed to create shared directory: " << directory << "\n";
            return false;
        }
    } else if (!(info.st_mode & S_IFDIR)) {
        std::cerr << "[Server] Path exists but is not a directory: " << directory << "\n";
        return false;
    }

    sharedDirectory_ = directory;
    protocol_->setSharedDirectory(directory);

    if (verbose_) {
        std::cout << "[Server] Shared directory set to: " << sharedDirectory_ << "\n";
    }

    return true;
}

std::string Server::getSharedDirectory() const {
    return sharedDirectory_;
}

void Server::setMaxConnections(size_t maxConnections) {
    maxConnections_ = maxConnections;
    
    if (verbose_) {
        std::cout << "[Server] Max connections set to: " 
                  << (maxConnections_ == 0 ? "unlimited" : std::to_string(maxConnections_)) << "\n";
    }
}

size_t Server::getMaxConnections() const {
    return maxConnections_;
}

void Server::setVerbose(bool enable) {
    verbose_ = enable;
    
    if (verbose_) {
        std::cout << "[Server] Verbose mode enabled\n";
    }
}

void Server::setTimeout(int seconds) {
    timeout_ = seconds;
    
    if (verbose_) {
        std::cout << "[Server] Timeout set to " << timeout_ << " seconds\n";
    }
}

const ServerMetrics& Server::getMetrics() const {
    return metrics_;
}

void Server::resetMetrics() {
    metrics_.reset();
    
    if (verbose_) {
        std::cout << "[Server] Metrics reset\n";
    }
}

bool Server::exportMetrics(const std::string& filename) const {
    try {
        metrics_.exportToCSV(filename);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Server] Error exporting metrics: " << e.what() << "\n";
        return false;
    }
}

void Server::displayMetrics() const {
    metrics_.display();
}

size_t Server::getActiveSessionCount() const {
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    return sessions_.size();
}

std::vector<std::string> Server::getActiveClients() const {
    std::vector<std::string> clients;
    
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    for (const auto& session : sessions_) {
        if (session && session->isActive()) {
            clients.push_back(session->getClientAddress());
        }
    }
    
    return clients;
}

void Server::acceptLoop() {
    std::cout << "[Server] Accepting client connections...\n";

    while (running_) {
        // Clean up finished sessions
        cleanupFinishedSessions();

        // Check if max connections reached
        if (maxConnections_ > 0 && getActiveSessionCount() >= maxConnections_) {
            if (verbose_) {
                std::cout << "[Server] Max connections reached, waiting...\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        // Accept new connection
        std::string clientAddr;
        int clientFd = socket_->acceptConnection(clientAddr);

        if (clientFd < 0) {
            // If server is stopping, exit gracefully
            if (!running_) {
                break;
            }
            // Otherwise, it's a real error
            if (running_) {
                metrics_.failedConnections++;
            }
            continue;
        }

        // Double-check we're still running
        if (!running_) {
            close(clientFd);
            break;
        }

        // Update metrics
        metrics_.incrementConnections();

        // Create and start new session
        auto session = std::make_unique<ClientSession>(clientFd, clientAddr, sharedDirectory_);
        session->start();

        {
            std::lock_guard<std::mutex> lock(sessionsMutex_);
            sessions_.push_back(std::move(session));
        }

        logEvent("Client connected: " + clientAddr);
    }
    
    if (verbose_) {
        std::cout << "[Server] Accept loop terminated\n";
    }
}

void Server::handleClient(int clientFd, const std::string& clientAddr) {
    (void)clientFd; // Unused - protocol processing is handled by ClientSession
    
    if (verbose_) {
        std::cout << "[Server] Handling client: " << clientAddr << "\n";
    }

    // Protocol processing is handled by ClientSession
    // This method can be used for additional per-client processing if needed
}

void Server::cleanupFinishedSessions() {
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    
    // Remove inactive sessions - destructor will handle thread cleanup
    auto it = std::remove_if(sessions_.begin(), sessions_.end(),
        [this](const std::unique_ptr<ClientSession>& session) {
            if (!session->isActive()) {
                metrics_.decrementActiveConnections();
                return true;
            }
            return false;
        });
    
    sessions_.erase(it, sessions_.end());
}

void Server::logEvent(const std::string& event) {
    if (verbose_) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::system_clock::to_time_t(now);
        std::cout << "[" << std::put_time(std::localtime(&timestamp), "%H:%M:%S") 
                  << "] " << event << "\n";
    }
}

bool Server::createSharedDirectory(const std::string& directory) {
    if (mkdir(directory.c_str(), 0755) == 0) {
        std::cout << "[Server] Created shared directory: " << directory << "\n";
        return true;
    }
    return false;
}