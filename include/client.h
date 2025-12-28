#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <memory>
#include "core/Client/client_metrics.h"
#include "core/Client/client_socket.h"
#include "core/Client/client_protocol.h"

/**
 * @class Client
 * @brief Main client class that wraps all client functionality
 * 
 * This class provides a high-level interface for file transfer operations,
 * combining socket management, protocol handling, and metrics tracking.
 */
class Client {
public:
    /**
     * @brief Constructor
     */
    Client();

    /**
     * @brief Destructor
     */
    ~Client();

    // Connection Management
    /**
     * @brief Connect to the server
     * @param ip Server IP address
     * @param port Server port number
     * @return true if connection successful, false otherwise
     */
    bool connect(const std::string& ip, uint16_t port);

    /**
     * @brief Disconnect from the server
     */
    void disconnect();

    /**
     * @brief Check if connected to server
     * @return true if connected, false otherwise
     */
    bool isConnected() const;

    // File Operations
    /**
     * @brief Request list of files from server
     * @return true if request successful, false otherwise
     */
    bool listFiles();

    /**
     * @brief Get list of files from server
     * @return Vector of filenames, empty if failed
     */
    std::vector<std::string> getFileList();

    /**
     * @brief Download a file from server
     * @param filename Name of file to download
     * @param saveDir Directory to save the file
     * @return true if download successful, false otherwise
     */
    bool getFile(const std::string& filename, const std::string& saveDir = ".");

    /**
     * @brief Upload a file to server
     * @param filepath Path to file to upload
     * @return true if upload successful, false otherwise
     */
    bool putFile(const std::string& filepath);

    // Metrics and Statistics
    /**
     * @brief Get current client metrics
     * @return ClientMetrics structure with current statistics
     */
    const ClientMetrics& getMetrics() const;

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

    // Configuration
    /**
     * @brief Set timeout for operations (in seconds)
     * @param seconds Timeout duration
     */
    void setTimeout(int seconds);

    /**
     * @brief Enable/disable verbose logging
     * @param enable true to enable, false to disable
     */
    void setVerbose(bool enable);

private:
    // Core components
    std::unique_ptr<ClientSocket> socket_;
    std::unique_ptr<ClientProtocol> protocol_;
    ClientMetrics metrics_;

    // Configuration
    int timeout_;
    bool verbose_;

    // Helper methods
    void updateMetrics();
    void logOperation(const std::string& operation, bool success);
};

#endif // CLIENT_H