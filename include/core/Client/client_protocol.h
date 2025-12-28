#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include <string> 
#include <vector>
#include "client_socket.h"

class ClientProtocol {
public: 
    explicit ClientProtocol(ClientSocket &socket);
    ~ClientProtocol() = default;

    void request_list();
    uint64_t request_get(const std::string &filename, 
                     const std::string &save_dir); 
    uint64_t request_put(const std::string &filepath);

    // New methods for GUI
    void sendListCommand(int sockfd);
    std::vector<std::string> receiveFileList(int sockfd);

private: 
    ClientSocket &socket_; 
};
#endif // CLIENT_PROTOCOL_H