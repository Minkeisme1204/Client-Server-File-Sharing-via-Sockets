#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include <string> 
#include "client_socket.h"
#include "client_metrics.h"

class ClientProtocol {
public: 
    explicit ClientProtocol(ClientSocket &socket);
    ~ClientProtocol() = default;
    
    void setMetrics(ClientMetrics* metrics);

    void request_list();
    void request_get(const std::string &filename, 
                     const std::string &save_dir); 
    void request_put(const std::string &filepath);

private: 
    ClientSocket &socket_;
    ClientMetrics* metrics_;
};
#endif // CLIENT_PROTOCOL_H