#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <string>
#include <memory>
#include <cstddef>

class ClientSocket {
public:
    ClientSocket();
    ~ClientSocket();

    bool connectToServer(const std::string& ip, uint16_t port);
    void disconnect();

    ssize_t sendData(const uint8_t* data, size_t size);
    ssize_t receiveData(uint8_t* buffer, size_t size);

    bool isConnected() const;
    int getSocketFd() const;

private:
    int socketFd_;
};


#endif // CLIENT_SOCKET_H