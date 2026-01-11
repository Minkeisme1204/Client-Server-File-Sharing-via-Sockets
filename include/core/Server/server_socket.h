#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <string>
#include <cstdint>
#include <memory>

/**
 * @class ServerSocket
 * @brief Manages server socket operations
 * 
 * Handles server socket creation, binding, listening,
 * and accepting client connections.
 */
class ServerSocket {
public:
    ServerSocket();
    ~ServerSocket();
    bool bind(uint16_t port, int backlog = 10);
    int acceptConnection(std::string& clientAddr);
    void close();
    bool isListening() const;
    int getSocketFd() const;
    static ssize_t sendData(int fd, const uint8_t* data, size_t size);
    static ssize_t receiveData(int fd, uint8_t* buffer, size_t size);

private:
    int socketFd_;
    uint16_t port_;
    bool listening_;
};

#endif // SERVER_SOCKET_H