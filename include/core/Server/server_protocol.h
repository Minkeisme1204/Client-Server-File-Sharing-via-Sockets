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
    void setSharedDirectory(const std::string& directory);
    void setSharedDirectoryPtr(std::shared_ptr<std::string> directoryPtr);
    void setMetrics(ServerMetrics* metrics);
    std::string getSharedDirectory() const;
    bool handleListCommand(int clientFd);
    bool handleGetCommand(int clientFd);
    bool handlePutCommand(int clientFd);
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