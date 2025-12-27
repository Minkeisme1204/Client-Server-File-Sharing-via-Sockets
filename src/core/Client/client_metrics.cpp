#include "client_metrics.h"
#include <fstream>
#include <iostream>
#include <iomanip>

void ClientMetrics::log_csv(const std::string &filename) const {
    // Check if file exists to determine if we need to write header
    std::ifstream checkFile(filename);
    bool fileExists = checkFile.good();
    checkFile.close();

    // Open file in append mode
    std::ofstream outFile(filename, std::ios::app);
    if (!outFile) {
        std::cerr << "[Metrics] Failed to open file: " << filename << "\n";
        return;
    }

    // Write CSV header if file is new
    if (!fileExists) {
        outFile << "RTT_ms,Throughput_kbps,Packet_Loss_Rate,Transfer_Latency_ms\n";
    }

    // Write metrics data
    outFile << std::fixed << std::setprecision(3)
            << rtt_ms << ","
            << throughput_kbps << ","
            << packet_loss_rate << ","
            << transfer_latency_ms << "\n";

    outFile.close();

    if (outFile.good()) {
        std::cout << "[Metrics] Logged to " << filename << "\n";
    } else {
        std::cerr << "[Metrics] Error writing to " << filename << "\n";
    }
}
