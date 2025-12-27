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

    /**
     * @brief Bind and listen on specified port
     * @param port Port number to listen on
     * @param backlog Maximum pending connections queue
     * @return true if successful, false otherwise
     */
    bool bind(uint16_t port, int backlog = 10);

    /**
     * @brief Accept a client connection
     * @param clientAddr Output parameter for client address
     * @return Client socket file descriptor, -1 on error
     */
    int acceptConnection(std::string& clientAddr);

    /**
     * @brief Close the server socket
     */
    void close();

    /**
     * @brief Check if socket is listening
     * @return true if listening, false otherwise
     */
    bool isListening() const;

    /**
     * @brief Get server socket file descriptor
     * @return Socket file descriptor
     */
    int getSocketFd() const;

    /**
     * @brief Send data through socket
     * @param fd Socket file descriptor
     * @param data Data buffer to send
     * @param size Size of data
     * @return Number of bytes sent, -1 on error
     */
    static ssize_t sendData(int fd, const uint8_t* data, size_t size);

    /**
     * @brief Receive data from socket
     * @param fd Socket file descriptor
     * @param buffer Buffer to receive data
     * @param size Size of buffer
     * @return Number of bytes received, -1 on error
     */
    static ssize_t receiveData(int fd, uint8_t* buffer, size_t size);

private:
    int socketFd_;
    uint16_t port_;
    bool listening_;
};

#endif // SERVER_SOCKET_H