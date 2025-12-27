#include "../include/server.h"
#include <iostream>
#include <string>
#include <csignal>
#include <thread>
#include <chrono>
#include <cstdlib>

// Global server instance for signal handling
Server* g_server = nullptr;

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\n\n[SERVER] Received shutdown signal. Stopping server...\n";
        if (g_server) {
            g_server->stop();
        }
        std::cout << "[SERVER] Cleanup completed. Exiting...\n";
        std::exit(0);
    }
}

void printBanner() {
    std::cout << "\n"
              << "╔═══════════════════════════════════════════════════════════╗\n"
              << "║          FILE TRANSFER SERVER - TEST APPLICATION          ║\n"
              << "╚═══════════════════════════════════════════════════════════╝\n"
              << std::endl;
}

void printHelp() {
    std::cout << "\n┌───────────────────────────────────────────────────────────┐\n";
    std::cout << "│ Available Commands:                                       │\n";
    std::cout << "├───────────────────────────────────────────────────────────┤\n";
    std::cout << "│  metrics     - Display current server metrics             │\n";
    std::cout << "│  clients     - Show active client connections             │\n";
    std::cout << "│  reset       - Reset server metrics                       │\n";
    std::cout << "│  export      - Export metrics to CSV file                 │\n";
    std::cout << "│  dir         - Change shared directory                    │\n";
    std::cout << "│  verbose     - Toggle verbose logging                     │\n";
    std::cout << "│  help        - Display this help menu                     │\n";
    std::cout << "│  quit/exit   - Stop server and exit                       │\n";
    std::cout << "└───────────────────────────────────────────────────────────┘\n";
    std::cout << std::endl;
}

void displayStatus(const Server& server) {
    std::cout << "\n┌───────────────────────── SERVER STATUS ───────────────────────────┐\n";
    std::cout << "│ Status: " << (server.isRunning() ? "RUNNING ✓" : "STOPPED ✗") << "\n";
    std::cout << "│ Shared Directory: " << server.getSharedDirectory() << "\n";
    std::cout << "│ Active Connections: " << server.getActiveSessionCount() << "\n";
    std::cout << "│ Max Connections: " << (server.getMaxConnections() == 0 ? "Unlimited" : std::to_string(server.getMaxConnections())) << "\n";
    std::cout << "└───────────────────────────────────────────────────────────────────┘\n";
}

int main(int argc, char* argv[]) {
    // Default configuration
    uint16_t port = 8080;
    std::string sharedDir = "./shared";
    bool verbose = true;

    // Parse command line arguments
    if (argc >= 2) {
        try {
            port = static_cast<uint16_t>(std::stoi(argv[1]));
        } catch (...) {
            std::cerr << "[ERROR] Invalid port number: " << argv[1] << std::endl;
            return 1;
        }
    }
    if (argc >= 3) {
        sharedDir = argv[2];
    }

    printBanner();

    // Create server instance
    Server server;
    g_server = &server;

    // Setup signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Configure server
    server.setVerbose(verbose);
    server.setMaxConnections(10); // Allow up to 10 concurrent connections
    server.setTimeout(300); // 5 minutes timeout

    std::cout << "[SERVER] Starting file transfer server...\n";
    std::cout << "[CONFIG] Port: " << port << "\n";
    std::cout << "[CONFIG] Shared Directory: " << sharedDir << "\n";
    std::cout << "[CONFIG] Verbose Mode: " << (verbose ? "ON" : "OFF") << "\n";
    std::cout << std::endl;

    // Start server
    if (!server.start(port, sharedDir)) {
        std::cerr << "[ERROR] Failed to start server on port " << port << std::endl;
        std::cerr << "[TIP] Make sure the port is not already in use.\n";
        std::cerr << "[TIP] Try using a different port: ./server_test <port> [shared_dir]\n";
        return 1;
    }

    std::cout << "[SUCCESS] Server started successfully!\n";
    std::cout << "[INFO] Server is listening on port " << port << "\n";
    std::cout << "[INFO] Shared directory: " << sharedDir << "\n";
    std::cout << "[INFO] Press Ctrl+C to stop the server\n";
    
    displayStatus(server);
    printHelp();

    // Run server in a separate thread
    std::thread serverThread([&server]() {
        server.run();
    });
    serverThread.detach(); // Detach so we don't block on join

    // Command loop
    std::string command;
    bool running = true;

    std::cout << "\nServer> " << std::flush;
    while (running && std::getline(std::cin, command)) {
        // Trim whitespace
        command.erase(0, command.find_first_not_of(" \t\r\n"));
        command.erase(command.find_last_not_of(" \t\r\n") + 1);

        if (command.empty()) {
            std::cout << "Server> " << std::flush;
            continue;
        }

        if (command == "quit" || command == "exit") {
            std::cout << "\n[SERVER] Shutting down...\n";
            running = false;
            server.stop();
            break;
        }
        else if (command == "help") {
            printHelp();
        }
        else if (command == "metrics") {
            std::cout << "\n";
            server.displayMetrics();
            std::cout << "\n";
        }
        else if (command == "clients") {
            auto clients = server.getActiveClients();
            std::cout << "\n┌─────────────── ACTIVE CLIENTS ───────────────┐\n";
            if (clients.empty()) {
                std::cout << "│ No active clients connected                  │\n";
            } else {
                for (size_t i = 0; i < clients.size(); ++i) {
                    std::cout << "│ [" << (i+1) << "] " << clients[i] << "\n";
                }
            }
            std::cout << "└──────────────────────────────────────────────┘\n\n";
        }
        else if (command == "reset") {
            server.resetMetrics();
            std::cout << "[INFO] Metrics have been reset.\n\n";
        }
        else if (command == "export") {
            std::string filename;
            std::cout << "Enter CSV filename: ";
            std::getline(std::cin, filename);
            if (server.exportMetrics(filename)) {
                std::cout << "[SUCCESS] Metrics exported to " << filename << "\n\n";
            } else {
                std::cout << "[ERROR] Failed to export metrics.\n\n";
            }
        }
        else if (command == "dir") {
            std::string newDir;
            std::cout << "Enter new shared directory path: ";
            std::getline(std::cin, newDir);
            if (server.setSharedDirectory(newDir)) {
                std::cout << "[SUCCESS] Shared directory changed to: " << newDir << "\n\n";
            } else {
                std::cout << "[ERROR] Failed to change directory. Make sure it exists.\n\n";
            }
        }
        else if (command == "verbose") {
            verbose = !verbose;
            server.setVerbose(verbose);
            std::cout << "[INFO] Verbose mode: " << (verbose ? "ON" : "OFF") << "\n\n";
        }
        else if (command == "status") {
            displayStatus(server);
        }
        else {
            std::cout << "[ERROR] Unknown command: '" << command << "'\n";
            std::cout << "[TIP] Type 'help' to see available commands.\n\n";
        }

        if (running) {
            std::cout << "Server> " << std::flush;
        }
    }

    // Cleanup - stop server
    server.stop();
    
    // Give threads time to cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "\n[SERVER] Server stopped. Goodbye!\n\n";

    return 0;
}
