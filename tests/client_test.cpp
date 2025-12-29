#include "../include/client.h"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

void printBanner() {
    std::cout << "\n"
              << "╔═══════════════════════════════════════════════════════════╗\n"
              << "║          FILE TRANSFER CLIENT - TEST APPLICATION          ║\n"
              << "╚═══════════════════════════════════════════════════════════╝\n"
              << std::endl;
}

void printHelp() {
    std::cout << "\n┌───────────────────────────────────────────────────────────┐\n";
    std::cout << "│ Available Commands:                                       │\n";
    std::cout << "├───────────────────────────────────────────────────────────┤\n";
    std::cout << "│  connect <ip> <port> - Connect to server                  │\n";
    std::cout << "│  disconnect          - Disconnect from server             │\n";
    std::cout << "│  list                - List files on server               │\n";
    std::cout << "│  get <filename>      - Download file from server          │\n";
    std::cout << "│  put <filepath>      - Upload file to server              │\n";
    std::cout << "│  metrics             - Display client metrics             │\n";
    std::cout << "│  history [limit]     - Display request history            │\n";
    std::cout << "│  reset               - Reset metrics                      │\n";
    std::cout << "│  export              - Export metrics to CSV              │\n";
    std::cout << "│  status              - Show connection status             │\n";
    std::cout << "│  help                - Display this help menu             │\n";
    std::cout << "│  quit/exit           - Exit the application               │\n";
    std::cout << "└───────────────────────────────────────────────────────────┘\n";
    std::cout << std::endl;
}

void displayStatus(const Client& client) {
    std::cout << "\n┌─────────────── CONNECTION STATUS ───────────────┐\n";
    std::cout << "│ Status: " << (client.isConnected() ? "CONNECTED ✓" : "DISCONNECTED ✗") << "\n";
    std::cout << "└─────────────────────────────────────────────────┘\n\n";
}

std::vector<std::string> splitCommand(const std::string& command) {
    std::vector<std::string> tokens;
    std::istringstream iss(command);
    std::string token;
    
    while (iss >> std::quoted(token)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

void handleConnect(Client& client, const std::vector<std::string>& args) {
    if (args.size() < 3) {
        std::cout << "[ERROR] Usage: connect <ip> <port>\n";
        std::cout << "[EXAMPLE] connect 127.0.0.1 8080\n\n";
        return;
    }

    if (client.isConnected()) {
        std::cout << "[WARNING] Already connected to server. Disconnect first.\n\n";
        return;
    }

    std::string ip = args[1];
    uint16_t port;
    
    try {
        port = static_cast<uint16_t>(std::stoi(args[2]));
    } catch (...) {
        std::cout << "[ERROR] Invalid port number: " << args[2] << "\n\n";
        return;
    }

    std::cout << "[INFO] Connecting to " << ip << ":" << port << "...\n";
    
    if (client.connect(ip, port)) {
        std::cout << "[SUCCESS] Connected to server!\n\n";
    } else {
        std::cout << "[ERROR] Failed to connect to server.\n";
        std::cout << "[TIP] Make sure the server is running and reachable.\n\n";
    }
}

void handleDisconnect(Client& client) {
    if (!client.isConnected()) {
        std::cout << "[INFO] Not connected to any server.\n\n";
        return;
    }

    std::cout << "[INFO] Disconnecting from server...\n";
    client.disconnect();
    std::cout << "[SUCCESS] Disconnected.\n\n";
}

void handleList(Client& client) {
    if (!client.isConnected()) {
        std::cout << "[ERROR] Not connected to server. Use 'connect' first.\n\n";
        return;
    }

    std::cout << "[INFO] Requesting file list from server...\n\n";
    
    if (client.listFiles()) {
        std::cout << "\n[SUCCESS] File list retrieved successfully.\n\n";
    } else {
        std::cout << "\n[ERROR] Failed to retrieve file list.\n\n";
    }
}

void handleGet(Client& client, const std::vector<std::string>& args) {
    if (!client.isConnected()) {
        std::cout << "[ERROR] Not connected to server. Use 'connect' first.\n\n";
        return;
    }

    if (args.size() < 2) {
        std::cout << "[ERROR] Usage: get <filename> [save_directory]\n";
        std::cout << "[EXAMPLE] get example.txt\n";
        std::cout << "[EXAMPLE] get example.txt ./downloads\n\n";
        return;
    }

    std::string filename = args[1];
    std::string saveDir = (args.size() >= 3) ? args[2] : ".";

    std::cout << "[INFO] Downloading file: " << filename << "\n";
    std::cout << "[INFO] Save directory: " << saveDir << "\n";
    
    if (client.getFile(filename, saveDir)) {
        std::cout << "[SUCCESS] File downloaded successfully!\n";
        std::cout << "[INFO] Saved to: " << saveDir << "/" << filename << "\n\n";
    } else {
        std::cout << "[ERROR] Failed to download file.\n";
        std::cout << "[TIP] Make sure the file exists on the server.\n\n";
    }
}

void handlePut(Client& client, const std::vector<std::string>& args) {
    if (!client.isConnected()) {
        std::cout << "[ERROR] Not connected to server. Use 'connect' first.\n\n";
        return;
    }

    if (args.size() < 2) {
        std::cout << "[ERROR] Usage: put <filepath>\n";
        std::cout << "[EXAMPLE] put ./myfile.txt\n";
        std::cout << "[EXAMPLE] put \"/path/to/my file.txt\"\n\n";
        return;
    }

    std::string filepath = args[1];

    // Check if file exists
    if (!fs::exists(filepath)) {
        std::cout << "[ERROR] File not found: " << filepath << "\n\n";
        return;
    }

    std::cout << "[INFO] Uploading file: " << filepath << "\n";
    std::cout << "[INFO] File size: " << fs::file_size(filepath) << " bytes\n";
    
    if (client.putFile(filepath)) {
        std::cout << "[SUCCESS] File uploaded successfully!\n\n";
    } else {
        std::cout << "[ERROR] Failed to upload file.\n\n";
    }
}

void handleMetrics(Client& client) {
    std::cout << "\n";
    client.displayMetrics();
    std::cout << "\n";
}

void handleReset(Client& client) {
    client.resetMetrics();
    std::cout << "[INFO] Metrics have been reset.\n\n";
}

void handleExport(Client& client) {
    std::string filename;
    std::cout << "Enter CSV filename: ";
    std::getline(std::cin, filename);
    
    if (filename.empty()) {
        filename = "client_metrics.csv";
        std::cout << "[INFO] Using default filename: " << filename << "\n";
    }
    
    if (client.exportMetrics(filename)) {
        std::cout << "[SUCCESS] Metrics exported to " << filename << "\n\n";
    } else {
        std::cout << "[ERROR] Failed to export metrics.\n\n";
    }
}

void handleHistory(Client& client, const std::vector<std::string>& args) {
    size_t limit = 20; // default
    
    if (args.size() >= 2) {
        try {
            limit = std::stoul(args[1]);
        } catch (...) {
            std::cout << "[ERROR] Invalid limit: " << args[1] << "\n";
            std::cout << "[INFO] Using default limit: 20\n\n";
        }
    }
    
    std::cout << "\n";
    client.displayHistory(limit);
    std::cout << "\n";
}

void runInteractiveMode(Client& client) {
    std::string input;
    bool running = true;

    std::cout << "\nClient> " << std::flush;
    while (running && std::getline(std::cin, input)) {
        // Trim whitespace
        input.erase(0, input.find_first_not_of(" \t\r\n"));
        input.erase(input.find_last_not_of(" \t\r\n") + 1);

        if (input.empty()) {
            std::cout << "Client> " << std::flush;
            continue;
        }

        auto args = splitCommand(input);
        std::string command = args[0];

        // Convert command to lowercase for case-insensitive matching
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);

        if (command == "quit" || command == "exit") {
            std::cout << "\n[CLIENT] Exiting...\n";
            if (client.isConnected()) {
                handleDisconnect(client);
            }
            running = false;
        }
        else if (command == "help") {
            printHelp();
        }
        else if (command == "connect") {
            handleConnect(client, args);
        }
        else if (command == "disconnect") {
            handleDisconnect(client);
        }
        else if (command == "list") {
            handleList(client);
        }
        else if (command == "get") {
            handleGet(client, args);
        }
        else if (command == "put") {
            handlePut(client, args);
        }
        else if (command == "metrics") {
            handleMetrics(client);
        }
        else if (command == "history") {
            handleHistory(client, args);
        }
        else if (command == "reset") {
            handleReset(client);
        }
        else if (command == "export") {
            handleExport(client);
        }
        else if (command == "status") {
            displayStatus(client);
        }
        else {
            std::cout << "[ERROR] Unknown command: '" << command << "'\n";
            std::cout << "[TIP] Type 'help' to see available commands.\n\n";
        }

        if (running) {
            std::cout << "Client> " << std::flush;
        }
    }
}

int main(int argc, char* argv[]) {
    printBanner();

    Client client;
    client.setTimeout(300); // 5 minutes timeout

    // Check for command line arguments for quick connection
    if (argc >= 3) {
        std::string ip = argv[1];
        uint16_t port;
        
        try {
            port = static_cast<uint16_t>(std::stoi(argv[2]));
        } catch (...) {
            std::cerr << "[ERROR] Invalid port number: " << argv[2] << std::endl;
            return 1;
        }

        std::cout << "[INFO] Auto-connecting to " << ip << ":" << port << "...\n";
        if (client.connect(ip, port)) {
            std::cout << "[SUCCESS] Connected to server!\n";
        } else {
            std::cout << "[ERROR] Failed to connect to server.\n";
            std::cout << "[TIP] Make sure the server is running.\n";
        }
    }

    std::cout << "\n[INFO] File Transfer Client Ready\n";
    std::cout << "[TIP] Type 'help' for available commands\n";
    
    if (!client.isConnected()) {
        std::cout << "[TIP] Use 'connect <ip> <port>' to connect to a server\n";
    }

    displayStatus(client);
    printHelp();

    // Run interactive command loop
    runInteractiveMode(client);

    std::cout << "\n[CLIENT] Goodbye!\n\n";

    return 0;
}
