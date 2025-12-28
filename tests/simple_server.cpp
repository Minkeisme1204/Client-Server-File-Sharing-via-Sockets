// server.cpp
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

#define PORT 8080

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    listen(server_fd, 5);
    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);

        int client_fd = accept(server_fd, (sockaddr*)&client_addr, &len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        std::cout << "Client connected" << std::endl;

        char buffer[1024];
        while (true) {
            ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
            if (n <= 0) {
                std::cout << "Client disconnected" << std::endl;
                break;
            }
            buffer[n] = '\0';
            std::cout << "Received: " << buffer << std::endl;
        }

        close(client_fd); // QUAN TRỌNG
    }

    close(server_fd);
    return 0;
}
