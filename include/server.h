#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include "core/Server/server_metrics.h"
#include "core/Server/server_socket.h"
#include "core/Server/server_protocol.h"
#include "core/Server/client_session.h"

/**
 * @class Server
 * @brief Main server class that wraps all server functionality
 * 
 * This class provides a high-level interface for running a file transfer server,
 * managing client connections, handling protocol operations, and tracking metrics.
 * Compatible with the Client class for seamless file transfer operations.
 */
class Server {
public:
    /**
     * @brief Constructor
     */
    Server();

    /**
     * @brief Destructor
     */
    ~Server();

    // Server Lifecycle
    /**
     * @brief Start the server on specified port
     * @param port Port number to listen on
     * @param sharedDir Directory for file sharing
     * @return true if server started successfully, false otherwise
     */
    bool start(uint16_t port, const std::string& sharedDir = "./shared");

    /**
     * @brief Stop the server and cleanup
     */
    void stop();

    /**
     * @brief Check if server is running
     * @return true if running, false otherwise
     */
    bool isRunning() const;

    /**
     * @brief Run the server (blocking call)
     * 
     * This method will block and continuously accept client connections
     * until stop() is called or an error occurs.
     */
    void run();

    // Configuration
    /**
     * @brief Set the shared directory
     * @param directory Path to shared directory
     * @return true if directory exists and is accessible
     */
    bool setSharedDirectory(const std::string& directory);

    /**
     * @brief Get the shared directory
     * @return Shared directory path
     */
    std::string getSharedDirectory() const;

    /**
     * @brief Set maximum number of concurrent connections
     * @param maxConnections Maximum connections (0 = unlimited)
     */
    void setMaxConnections(size_t maxConnections);

    /**
     * @brief Get maximum connections limit
     * @return Maximum connections
     */
    size_t getMaxConnections() const;

    /**
     * @brief Enable/disable verbose logging
     * @param enable true to enable, false to disable
     */
    void setVerbose(bool enable);

    /**
     * @brief Set connection timeout
     * @param seconds Timeout in seconds
     */
    void setTimeout(int seconds);

    // Metrics and Statistics
    /**
     * @brief Get current server metrics
     * @return ServerMetrics structure with current statistics
     */
    const ServerMetrics& getMetrics() const;

    /**
     * @brief Reset all metrics to zero
     */
    void resetMetrics();

    /**
     * @brief Export metrics to CSV file
     * @param filename CSV file path
     * @return true if export successful, false otherwise
     */
    bool exportMetrics(const std::string& filename) const;

    /**
     * @brief Display metrics to console
     */
    void displayMetrics() const;

    /**
     * @brief Get number of active client sessions
     * @return Number of active sessions
     */
    size_t getActiveSessionCount() const;

    /**
     * @brief Get list of active client addresses
     * @return Vector of client address strings
     */
    std::vector<std::string> getActiveClients() const;

private:
    // Core components
    std::unique_ptr<ServerSocket> socket_;
    std::unique_ptr<ServerProtocol> protocol_;
    ServerMetrics metrics_;

    // Session management
    std::vector<std::unique_ptr<ClientSession>> sessions_;
    mutable std::mutex sessionsMutex_;

    // Server state
    std::atomic<bool> running_;
    std::string sharedDirectory_;
    uint16_t port_;
    size_t maxConnections_;
    int timeout_;
    bool verbose_;

    // Accept thread
    std::unique_ptr<std::thread> acceptThread_;

    // Helper methods
    void acceptLoop();
    void handleClient(int clientFd, const std::string& clientAddr);
    void cleanupFinishedSessions();
    void logEvent(const std::string& event);
    bool createSharedDirectory(const std::string& directory);
};

#endif // SERVER_H