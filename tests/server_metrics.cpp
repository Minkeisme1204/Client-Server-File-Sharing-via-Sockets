/**
 * Server Metrics Monitor
 * 
 * Monitors server-side metrics including:
 * - Number of concurrent connections
 * - CPU and Memory usage
 * - Network throughput
 * - Thread count
 * 
 * This program runs alongside the server and periodically collects metrics.
 * Results are logged to CSV for analysis.
 * 
 * Usage: ./server_metrics <server_pid> <sampling_interval_ms> [output_csv] [duration_seconds]
 * Example: ./server_metrics 12345 1000 server_metrics.csv 300
 */

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <thread>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <dirent.h>

using namespace std;
using namespace std::chrono;

struct ServerMetrics {
    string timestamp{""};
    
    // Process info
    int pid{0};
    int numThreads{0};
    
    // CPU metrics
    double cpuPercent{0.0};
    unsigned long long cpuTime{0};  // in jiffies
    
    // Memory metrics
    uint64_t memoryRssKB{0};      // Resident Set Size
    uint64_t memoryVmsKB{0};      // Virtual Memory Size
    double memoryPercent{0.0};
    
    // Network metrics (estimated from /proc/net/dev)
    uint64_t bytesReceived{0};
    uint64_t bytesSent{0};
    double rxThroughputMBps{0.0};
    double txThroughputMBps{0.0};
    
    // File descriptors (proxy for connections)
    int numFileDescriptors{0};
    int numSockets{0};
};

static bool keepRunning = true;

void signalHandler(int signum) {
    cout << "\nInterrupt signal received. Stopping monitoring..." << endl;
    keepRunning = false;
}

/**
 * Get current timestamp as string
 */
string getCurrentTimestamp() {
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    auto timer = system_clock::to_time_t(now);
    
    ostringstream oss;
    oss << put_time(localtime(&timer), "%Y-%m-%d %H:%M:%S");
    oss << '.' << setfill('0') << setw(3) << ms.count();
    return oss.str();
}

/**
 * Check if process exists
 */
bool processExists(int pid) {
    return (kill(pid, 0) == 0);
}

/**
 * Get total system memory in KB
 */
uint64_t getTotalSystemMemoryKB() {
    ifstream meminfo("/proc/meminfo");
    string line;
    while (getline(meminfo, line)) {
        if (line.find("MemTotal:") == 0) {
            istringstream iss(line);
            string label;
            uint64_t value;
            iss >> label >> value;
            return value;
        }
    }
    return 0;
}

/**
 * Get number of CPU cores
 */
int getNumCPUCores() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

/**
 * Read process statistics from /proc/[pid]/stat
 */
bool readProcStat(int pid, ServerMetrics& metrics) {
    string statPath = "/proc/" + to_string(pid) + "/stat";
    ifstream statFile(statPath);
    
    if (!statFile.is_open()) {
        return false;
    }
    
    string line;
    getline(statFile, line);
    
    // Parse stat file (fields are space-separated)
    // We need: pid, comm, state, ..., utime(14), stime(15), ..., num_threads(20)
    istringstream iss(line);
    string token;
    vector<string> tokens;
    
    // Special handling for comm field which can contain spaces
    int parenCount = 0;
    string currentToken;
    for (char c : line) {
        if (c == '(') parenCount++;
        if (c == ')') parenCount--;
        
        if (c == ' ' && parenCount == 0) {
            if (!currentToken.empty()) {
                tokens.push_back(currentToken);
                currentToken.clear();
            }
        } else {
            currentToken += c;
        }
    }
    if (!currentToken.empty()) {
        tokens.push_back(currentToken);
    }
    
    if (tokens.size() >= 20) {
        unsigned long long utime = stoull(tokens[13]);
        unsigned long long stime = stoull(tokens[14]);
        metrics.cpuTime = utime + stime;
        metrics.numThreads = stoi(tokens[19]);
    }
    
    return true;
}

/**
 * Read memory statistics from /proc/[pid]/status
 */
bool readProcStatus(int pid, ServerMetrics& metrics) {
    string statusPath = "/proc/" + to_string(pid) + "/status";
    ifstream statusFile(statusPath);
    
    if (!statusFile.is_open()) {
        return false;
    }
    
    string line;
    while (getline(statusFile, line)) {
        if (line.find("VmRSS:") == 0) {
            istringstream iss(line);
            string label;
            uint64_t value;
            iss >> label >> value;
            metrics.memoryRssKB = value;
        } else if (line.find("VmSize:") == 0) {
            istringstream iss(line);
            string label;
            uint64_t value;
            iss >> label >> value;
            metrics.memoryVmsKB = value;
        }
    }
    
    return true;
}

/**
 * Count file descriptors for a process
 */
int countFileDescriptors(int pid) {
    string fdPath = "/proc/" + to_string(pid) + "/fd";
    int count = 0;
    
    DIR* dir = opendir(fdPath.c_str());
    if (!dir) return 0;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name != "." && name != "..") {
            count++;
        }
    }
    closedir(dir);
    
    return count;
}

/**
 * Count sockets for a process
 */
int countSockets(int pid) {
    string fdPath = "/proc/" + to_string(pid) + "/fd";
    int count = 0;
    
    DIR* dir = opendir(fdPath.c_str());
    if (!dir) return 0;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name == "." || name == "..") continue;
        
        string linkPath = fdPath + "/" + name;
        char buffer[256];
        ssize_t len = readlink(linkPath.c_str(), buffer, sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            string target(buffer);
            if (target.find("socket:") == 0) {
                count++;
            }
        }
    }
    closedir(dir);
    
    return count;
}

/**
 * Read network statistics from /proc/net/dev
 */
bool readNetworkStats(uint64_t& bytesRx, uint64_t& bytesTx) {
    ifstream netdev("/proc/net/dev");
    if (!netdev.is_open()) {
        return false;
    }
    
    string line;
    // Skip header lines
    getline(netdev, line);
    getline(netdev, line);
    
    bytesRx = 0;
    bytesTx = 0;
    
    while (getline(netdev, line)) {
        // Skip loopback interface
        if (line.find("lo:") != string::npos) {
            continue;
        }
        
        // Parse line: interface: rx_bytes ... tx_bytes ...
        size_t colonPos = line.find(':');
        if (colonPos == string::npos) continue;
        
        string stats = line.substr(colonPos + 1);
        istringstream iss(stats);
        
        uint64_t rx, tx;
        iss >> rx;  // bytes received
        
        // Skip 7 fields to get to bytes transmitted
        for (int i = 0; i < 7; i++) {
            uint64_t dummy;
            iss >> dummy;
        }
        iss >> tx;  // bytes transmitted
        
        bytesRx += rx;
        bytesTx += tx;
    }
    
    return true;
}

/**
 * Collect all metrics for the server process
 */
ServerMetrics collectMetrics(int pid, const ServerMetrics& previousMetrics, double intervalSec) {
    ServerMetrics metrics;
    metrics.timestamp = getCurrentTimestamp();
    metrics.pid = pid;
    
    // Read process stats
    if (!readProcStat(pid, metrics)) {
        cerr << "Warning: Could not read process stats" << endl;
        return metrics;
    }
    
    // Read memory stats
    if (!readProcStatus(pid, metrics)) {
        cerr << "Warning: Could not read process status" << endl;
        return metrics;
    }
    
    // Calculate CPU percentage
    if (previousMetrics.cpuTime > 0 && intervalSec > 0) {
        unsigned long long cpuDelta = metrics.cpuTime - previousMetrics.cpuTime;
        int numCores = getNumCPUCores();
        // Convert jiffies to seconds (assuming 100 jiffies per second)
        double cpuTimeSec = cpuDelta / 100.0;
        metrics.cpuPercent = (cpuTimeSec / intervalSec) * 100.0;
        
        // Normalize to single core percentage (optional)
        // metrics.cpuPercent /= numCores;
    }
    
    // Calculate memory percentage
    uint64_t totalMemKB = getTotalSystemMemoryKB();
    if (totalMemKB > 0) {
        metrics.memoryPercent = (static_cast<double>(metrics.memoryRssKB) / totalMemKB) * 100.0;
    }
    
    // Read network stats
    uint64_t totalRx, totalTx;
    if (readNetworkStats(totalRx, totalTx)) {
        metrics.bytesReceived = totalRx;
        metrics.bytesSent = totalTx;
        
        // Calculate throughput
        if (previousMetrics.bytesReceived > 0 && intervalSec > 0) {
            uint64_t rxDelta = totalRx - previousMetrics.bytesReceived;
            uint64_t txDelta = totalTx - previousMetrics.bytesSent;
            
            metrics.rxThroughputMBps = (rxDelta / intervalSec) / (1024.0 * 1024.0);
            metrics.txThroughputMBps = (txDelta / intervalSec) / (1024.0 * 1024.0);
        }
    }
    
    // Count file descriptors and sockets
    metrics.numFileDescriptors = countFileDescriptors(pid);
    metrics.numSockets = countSockets(pid);
    
    return metrics;
}

/**
 * Write CSV header
 */
void writeCSVHeader(ofstream& file) {
    file << "Timestamp,PID,NumThreads,";
    file << "CPUPercent,MemoryRssKB,MemoryVmsKB,MemoryPercent,";
    file << "RxThroughputMBps,TxThroughputMBps,";
    file << "NumFileDescriptors,NumSockets" << endl;
}

/**
 * Write metrics to CSV
 */
void writeMetricsToCSV(ofstream& file, const ServerMetrics& metrics) {
    file << metrics.timestamp << ","
         << metrics.pid << ","
         << metrics.numThreads << ","
         << fixed << setprecision(2) << metrics.cpuPercent << ","
         << metrics.memoryRssKB << ","
         << metrics.memoryVmsKB << ","
         << fixed << setprecision(2) << metrics.memoryPercent << ","
         << fixed << setprecision(3) << metrics.rxThroughputMBps << ","
         << fixed << setprecision(3) << metrics.txThroughputMBps << ","
         << metrics.numFileDescriptors << ","
         << metrics.numSockets << endl;
}

/**
 * Print metrics to console
 */
void printMetrics(const ServerMetrics& metrics) {
    cout << "\r[" << metrics.timestamp << "] "
         << "Threads: " << metrics.numThreads << " | "
         << "CPU: " << fixed << setprecision(1) << metrics.cpuPercent << "% | "
         << "Mem: " << (metrics.memoryRssKB / 1024) << " MB | "
         << "Sockets: " << metrics.numSockets << " | "
         << "RX: " << fixed << setprecision(2) << metrics.rxThroughputMBps << " MB/s | "
         << "TX: " << fixed << setprecision(2) << metrics.txThroughputMBps << " MB/s   ";
    cout.flush();
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <server_pid> <sampling_interval_ms> [output_csv] [duration_seconds]" << endl;
        cerr << "Example: " << argv[0] << " 12345 1000 server_metrics.csv 300" << endl;
        return 1;
    }
    
    int serverPid = stoi(argv[1]);
    int samplingIntervalMs = stoi(argv[2]);
    string outputCsv = (argc >= 4) ? argv[3] : "server_metrics_" + to_string(time(nullptr)) + ".csv";
    int durationSeconds = (argc >= 5) ? stoi(argv[4]) : 0;  // 0 means run indefinitely
    
    cout << "=== Server Metrics Monitor ===" << endl;
    cout << "Server PID: " << serverPid << endl;
    cout << "Sampling Interval: " << samplingIntervalMs << " ms" << endl;
    cout << "Output CSV: " << outputCsv << endl;
    if (durationSeconds > 0) {
        cout << "Duration: " << durationSeconds << " seconds" << endl;
    } else {
        cout << "Duration: Indefinite (press Ctrl+C to stop)" << endl;
    }
    cout << "============================\n" << endl;
    
    // Check if process exists
    if (!processExists(serverPid)) {
        cerr << "Error: Process with PID " << serverPid << " does not exist" << endl;
        return 1;
    }
    
    // Setup signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    // Open CSV file
    ofstream csvFile(outputCsv);
    if (!csvFile.is_open()) {
        cerr << "Error: Cannot open output CSV file: " << outputCsv << endl;
        return 1;
    }
    writeCSVHeader(csvFile);
    
    cout << "Monitoring started. Press Ctrl+C to stop...\n" << endl;
    
    ServerMetrics previousMetrics;
    auto startTime = steady_clock::now();
    
    // Monitoring loop
    while (keepRunning) {
        // Check if duration limit reached
        if (durationSeconds > 0) {
            auto elapsed = duration_cast<seconds>(steady_clock::now() - startTime).count();
            if (elapsed >= durationSeconds) {
                cout << "\nDuration limit reached. Stopping..." << endl;
                break;
            }
        }
        
        // Check if process still exists
        if (!processExists(serverPid)) {
            cout << "\nServer process terminated. Stopping monitoring..." << endl;
            break;
        }
        
        // Collect metrics
        double intervalSec = samplingIntervalMs / 1000.0;
        ServerMetrics metrics = collectMetrics(serverPid, previousMetrics, intervalSec);
        
        // Write to CSV
        writeMetricsToCSV(csvFile, metrics);
        csvFile.flush();
        
        // Print to console
        printMetrics(metrics);
        
        // Save for next iteration
        previousMetrics = metrics;
        
        // Wait for next sample
        this_thread::sleep_for(milliseconds(samplingIntervalMs));
    }
    
    csvFile.close();
    
    cout << "\n\nMonitoring stopped. Results saved to: " << outputCsv << endl;
    
    return 0;
}
