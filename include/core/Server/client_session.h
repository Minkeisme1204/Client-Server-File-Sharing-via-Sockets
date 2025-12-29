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
    /**
     * @brief Constructor
     * @param clientFd Client socket file descriptor
     * @param clientAddr Client address information
     * @param sharedDir Shared directory path (shared pointer)
     * @param metrics Pointer to ServerMetrics
     */
    ClientSession(int clientFd, const std::string& clientAddr, std::shared_ptr<std::string> sharedDir, ServerMetrics* metrics);

    /**
     * @brief Destructor
     */
    ~ClientSession();

    /**
     * @brief Start handling the client session
     */
    void start();

    /**
     * @brief Stop the client session
     */
    void stop();

    /**
     * @brief Check if session is active
     * @return true if active, false otherwise
     */
    bool isActive() const;

    /**
     * @brief Get client address
     * @return Client address string
     */
    std::string getClientAddress() const;

    /**
     * @brief Get session start time
     * @return Session start timestamp
     */
    std::chrono::system_clock::time_point getStartTime() const;

    /**
     * @brief Get bytes transferred
     * @return Total bytes sent and received
     */
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