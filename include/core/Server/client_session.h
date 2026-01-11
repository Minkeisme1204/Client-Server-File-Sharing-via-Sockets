#ifndef CLIENT_SESSION_H
#define CLIENT_SESSION_H

#include <string>
#include <thread>
#include <memory>
#include <atomic>
#include <chrono>
#include "server_metrics.h"

/**
 * @class ClientSession
 * @brief Represents a single client connection session
 * 
 * Manages the lifecycle of a client connection, including
 * socket handling, protocol processing, and metrics tracking.
 */
class ClientSession {
public:
    ClientSession(int clientFd, const std::string& clientAddr, std::shared_ptr<std::string> sharedDir, ServerMetrics* metrics);
    ~ClientSession();
    void start();
    void stop();
    bool isActive() const;
    std::string getClientAddress() const;
    std::chrono::system_clock::time_point getStartTime() const;
    size_t getBytesTransferred() const;

private:
    int clientFd_;
    std::string clientAddr_;
    std::shared_ptr<std::string> sharedDir_;
    ServerMetrics* metrics_;
    std::unique_ptr<std::thread> thread_;
    std::atomic<bool> active_;
    std::chrono::system_clock::time_point startTime_;
    std::atomic<size_t> bytesTransferred_;

    // Session handling
    void handleSession();
    void cleanup();
};

#endif // CLIENT_SESSION_H