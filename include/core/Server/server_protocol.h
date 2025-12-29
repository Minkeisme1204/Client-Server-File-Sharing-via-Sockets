#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include "server_metrics.h"

// Protocol command codes
#define CMD_LIST 0x01
#define CMD_GET  0x02
#define CMD_PUT  0x03

/**
 * @class ServerProtocol
 * @brief Handles server-side protocol operations
 * 
 * Processes client requests according to the file transfer protocol,
 * including LIST, GET, and PUT commands.
 */
class ServerProtocol {
public:
    ServerProtocol();
    ~ServerProtocol() = default;

    /**
     * @brief Set the shared directory for file operations
     * @param directory Path to shared directory
     */
    void setSharedDirectory(const std::string& directory);

    /**
     * @brief Set the shared directory pointer for dynamic updates
     * @param directoryPtr Shared pointer to directory path
     */
    void setSharedDirectoryPtr(std::shared_ptr<std::string> directoryPtr);

    /**
     * @brief Set metrics pointer for tracking
     * @param metrics Pointer to ServerMetrics
     */
    void setMetrics(ServerMetrics* metrics);

    /**
     * @brief Get the shared directory
     * @return Shared directory path
     */
    std::string getSharedDirectory() const;

    /**
     * @brief Handle LIST command - send list of files to client
     * @param clientFd Client socket file descriptor
     * @return true if successful, false otherwise
     */
    bool handleListCommand(int clientFd);

    /**
     * @brief Handle GET command - send file to client
     * @param clientFd Client socket file descriptor
     * @return true if successful, false otherwise
     */
    bool handleGetCommand(int clientFd);

    /**
     * @brief Handle PUT command - receive file from client
     * @param clientFd Client socket file descriptor
     * @return true if successful, false otherwise
     */
    bool handlePutCommand(int clientFd);

    /**
     * @brief Process client request
     * @param clientFd Client socket file descriptor
     * @return true if should continue session, false to terminate
     */
    bool processRequest(int clientFd);

private:
    std::shared_ptr<std::string> sharedDirectory_;
    ServerMetrics* metrics_;

    // Helper methods
    std::vector<std::string> listFiles();
    bool sendFile(int clientFd, const std::string& filename);
    bool receiveFile(int clientFd, const std::string& filename, uint64_t fileSize);
};

#endif // SERVER_PROTOCOL_H