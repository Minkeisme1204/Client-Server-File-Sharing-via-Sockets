#include "server_metrics.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>

ServerMetrics::ServerMetrics() {
    startTime = std::chrono::system_clock::now();
}

double ServerMetrics::getUptimeSeconds() const {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
    return duration.count();
}

void ServerMetrics::incrementConnections() {
    totalConnections++;
    activeConnections++;
}

void ServerMetrics::decrementActiveConnections() {
    if (activeConnections > 0) {
        activeConnections--;
    }
}

void ServerMetrics::addBytesReceived(uint64_t bytes) {
    totalBytesReceived += bytes;
}

void ServerMetrics::addBytesSent(uint64_t bytes) {
    totalBytesSent += bytes;
}

void ServerMetrics::updateThroughput(uint64_t bytes, double duration_ms) {
    if (duration_ms <= 0) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    // Calculate throughput in kbps
    double throughput = (bytes * 8.0) / (duration_ms / 1000.0) / 1024.0;
    
    // Update average (simple moving average)
    if (averageThroughput_kbps == 0.0) {
        averageThroughput_kbps = throughput;
    } else {
        averageThroughput_kbps = (averageThroughput_kbps * 0.9) + (throughput * 0.1);
    }
    
    // Update peak
    if (throughput > peakThroughput_kbps) {
        peakThroughput_kbps = throughput;
    }
}

void ServerMetrics::updateLatency(double latency_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Update average latency (exponential moving average)
    if (averageLatency_ms == 0.0) {
        averageLatency_ms = latency_ms;
    } else {
        averageLatency_ms = (averageLatency_ms * 0.9) + (latency_ms * 0.1);
    }
}

void ServerMetrics::reset() {
    totalConnections = 0;
    activeConnections = 0;
    failedConnections = 0;
    totalBytesReceived = 0;
    totalBytesSent = 0;
    filesUploaded = 0;
    filesDownloaded = 0;
    
    std::lock_guard<std::mutex> lock(mutex_);
    averageThroughput_kbps = 0.0;
    peakThroughput_kbps = 0.0;
    averageLatency_ms = 0.0;
    startTime = std::chrono::system_clock::now();
}

void ServerMetrics::exportToCSV(const std::string& filename) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if file exists
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
        outFile << "Timestamp,Uptime_s,Total_Connections,Active_Connections,Failed_Connections,"
                << "Bytes_Received,Bytes_Sent,Files_Uploaded,Files_Downloaded,"
                << "Avg_Throughput_kbps,Peak_Throughput_kbps,Avg_Latency_ms\n";
    }

    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);

    // Write metrics data
    outFile << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S") << ","
            << getUptimeSeconds() << ","
            << totalConnections.load() << ","
            << activeConnections.load() << ","
            << failedConnections.load() << ","
            << totalBytesReceived.load() << ","
            << totalBytesSent.load() << ","
            << filesUploaded.load() << ","
            << filesDownloaded.load() << ","
            << std::fixed << std::setprecision(2)
            << averageThroughput_kbps << ","
            << peakThroughput_kbps << ","
            << averageLatency_ms << "\n";

    outFile.close();

    if (outFile.good()) {
        std::cout << "[Metrics] Logged to " << filename << "\n";
    } else {
        std::cerr << "[Metrics] Error writing to " << filename << "\n";
    }
}

void ServerMetrics::display() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::cout << "\n=== Server Metrics ===\n";
    std::cout << "Uptime:              " << getUptimeSeconds() << " seconds\n";
    std::cout << "Total Connections:   " << totalConnections.load() << "\n";
    std::cout << "Active Connections:  " << activeConnections.load() << "\n";
    std::cout << "Failed Connections:  " << failedConnections.load() << "\n";
    std::cout << "Bytes Received:      " << totalBytesReceived.load() << " bytes\n";
    std::cout << "Bytes Sent:          " << totalBytesSent.load() << " bytes\n";
    std::cout << "Files Uploaded:      " << filesUploaded.load() << "\n";
    std::cout << "Files Downloaded:    " << filesDownloaded.load() << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Avg Throughput:      " << averageThroughput_kbps << " kbps\n";
    std::cout << "Peak Throughput:     " << peakThroughput_kbps << " kbps\n";
    std::cout << "Avg Latency:         " << averageLatency_ms << " ms\n";
    std::cout << "=====================\n\n";
}