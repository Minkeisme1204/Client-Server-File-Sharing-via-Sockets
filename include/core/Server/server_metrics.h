#ifndef SERVER_METRICS_H
#define SERVER_METRICS_H

#include <string>
#include <atomic>
#include <chrono>
#include <mutex>

/**
 * @struct ServerMetrics
 * @brief Tracks server performance metrics
 * 
 * Collects and manages various server statistics including
 * connection counts, throughput, and resource usage.
 */
struct ServerMetrics {
    // Connection metrics
    std::atomic<uint64_t> totalConnections{0};
    std::atomic<uint64_t> activeConnections{0};
    std::atomic<uint64_t> failedConnections{0};

    // Transfer metrics
    std::atomic<uint64_t> totalBytesReceived{0};
    std::atomic<uint64_t> totalBytesSent{0};
    std::atomic<uint64_t> filesUploaded{0};
    std::atomic<uint64_t> filesDownloaded{0};

    // Performance metrics
    double averageThroughput_kbps = 0.0;
    double peakThroughput_kbps = 0.0;
    double averageLatency_ms = 0.0;

    // Server uptime
    std::chrono::system_clock::time_point startTime;

    /**
     * @brief Constructor - initializes start time
     */
    ServerMetrics();

    /**
     * @brief Get server uptime in seconds
     * @return Uptime in seconds
     */
    double getUptimeSeconds() const;

    /**
     * @brief Increment connection counter
     */
    void incrementConnections();

    /**
     * @brief Decrement active connections
     */
    void decrementActiveConnections();

    /**
     * @brief Add bytes to received counter
     * @param bytes Number of bytes received
     */
    void addBytesReceived(uint64_t bytes);

    /**
     * @brief Add bytes to sent counter
     * @param bytes Number of bytes sent
     */
    void addBytesSent(uint64_t bytes);

    /**
     * @brief Update throughput calculation
     * @param bytes Bytes transferred
     * @param duration_ms Duration in milliseconds
     */
    void updateThroughput(uint64_t bytes, double duration_ms);

    /**
     * @brief Reset all metrics to zero
     */
    void reset();

    /**
     * @brief Export metrics to CSV file
     * @param filename CSV file path
     */
    void exportToCSV(const std::string& filename) const;

    /**
     * @brief Display metrics to console
     */
    void display() const;

private:
    mutable std::mutex mutex_; // For thread-safe access to non-atomic members
};

#endif // SERVER_METRICS_H