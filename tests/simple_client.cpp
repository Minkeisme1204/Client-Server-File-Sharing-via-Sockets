// client.cpp
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

#define PORT 8080
#define SERVER_IP "127.0.0.1"

int connect_to_server() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

    if (connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    std::cout << "Connected to server" << std::endl;
    return sock;
}

int main() {
    // 🔹 Lần kết nối 1
    int sock = connect_to_server();
    std::cout << "Reconnected to server " << sock << std::endl;
    if (sock < 0) return 1;

    const char* msg1 = "Hello server (first connection)";
    send(sock, msg1, strlen(msg1), 0);

    std::cout << "Disconnecting..." << std::endl;
    close(sock);  // 🔴 NGẮT KẾT NỐI
    sock = -1; 

    sleep(3);     // giả lập chờ

    // 🔹 Lần kết nối 2
    sock = connect_to_server();
    std::cout << "Reconnected to server " << sock << std::endl;
    if (sock < 0) return 1;

    const char* msg2 = "Hello server (reconnected)";
    send(sock, msg2, strlen(msg2), 0);

    close(sock);
    return 0;
}
