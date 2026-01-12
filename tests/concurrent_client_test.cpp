/**
 * Concurrent Client Test - Scenario 1
 * 
 * Tests multiple concurrent clients connecting from potentially different machines.
 * Measures client-side metrics including throughput, RTT, and completion time.
 * Results are logged to CSV for analysis.
 * 
 * Usage: ./concurrent_client_test <server_ip> <port> <client_id> <num_iterations> <test_file> [output_csv]
 * Example: ./concurrent_client_test 127.0.0.1 8080 1 10 test_files/test_file.txt client_metrics.csv
 */

#include "../include/client.h"
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <iomanip>
#include <sys/stat.h>
#include <thread>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cmath>

using namespace std;
using namespace std::chrono;

struct ClientTestMetrics {
    int clientId{0};
    int iteration{0};
    string operation{"PUT"};
    string filename{""};
    uint64_t fileSize{0};
    
    // Timing metrics
    string startTime{""};
    string endTime{""};
    double durationMs{0.0};
    
    // Performance metrics
    double throughputMBps{0.0};
    double avgRttMs{0.0};
    
    // Network metrics
    uint64_t bytesSent{0};
    uint64_t bytesReceived{0};
    
    // Status
    bool success{false};
    string errorMsg{""};
};

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
 * Get file size in bytes
 */
uint64_t getFileSize(const string& filepath) {
    struct stat statbuf;
    if (stat(filepath.c_str(), &statbuf) == 0) {
        return statbuf.st_size;
    }
    return 0;
}

/**
 * Extract filename from path
 */
string extractFilename(const string& filepath) {
    size_t pos = filepath.find_last_of("/\\");
    if (pos != string::npos) {
        return filepath.substr(pos + 1);
    }
    return filepath;
}

/**
 * Measure RTT by sending a small LIST command
 */
double measureRTT(Client& client) {
    auto start = high_resolution_clock::now();
    client.listFiles();
    auto end = high_resolution_clock::now();
    
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

/**
 * Perform PUT operation and collect metrics
 */
ClientTestMetrics performPutTest(Client& client, const string& filepath, int clientId, int iteration) {
    ClientTestMetrics metrics;
    metrics.clientId = clientId;
    metrics.iteration = iteration;
    metrics.operation = "PUT";
    metrics.filename = extractFilename(filepath);
    metrics.fileSize = getFileSize(filepath);
    
    // Measure RTT before operation
    try {
        metrics.avgRttMs = measureRTT(client);
    } catch (...) {
        metrics.avgRttMs = 0.0;
    }
    
    // Perform PUT operation
    metrics.startTime = getCurrentTimestamp();
    auto start = high_resolution_clock::now();
    
    bool result = client.putFile(filepath);
    
    auto end = high_resolution_clock::now();
    metrics.endTime = getCurrentTimestamp();
    
    metrics.durationMs = duration_cast<microseconds>(end - start).count() / 1000.0;
    metrics.success = result;
    
    if (result) {
        // Calculate throughput
        double durationSec = metrics.durationMs / 1000.0;
        double sizeMB = static_cast<double>(metrics.fileSize) / (1024.0 * 1024.0);
        metrics.throughputMBps = (durationSec > 0) ? (sizeMB / durationSec) : 0.0;
        metrics.bytesSent = metrics.fileSize;
    } else {
        metrics.errorMsg = "PUT operation failed";
    }
    
    return metrics;
}

/**
 * Perform GET operation and collect metrics
 */
ClientTestMetrics performGetTest(Client& client, const string& filename, int clientId, int iteration) {
    ClientTestMetrics metrics;
    metrics.clientId = clientId;
    metrics.iteration = iteration;
    metrics.operation = "GET";
    metrics.filename = filename;
    
    // Measure RTT before operation
    try {
        metrics.avgRttMs = measureRTT(client);
    } catch (...) {
        metrics.avgRttMs = 0.0;
    }
    
    // Perform GET operation
    metrics.startTime = getCurrentTimestamp();
    auto start = high_resolution_clock::now();
    
    bool result = client.getFile(filename);
    
    auto end = high_resolution_clock::now();
    metrics.endTime = getCurrentTimestamp();
    
    metrics.durationMs = duration_cast<microseconds>(end - start).count() / 1000.0;
    metrics.success = result;
    
    if (result) {
        // Get downloaded file size
        string downloadPath = "downloads/" + filename;
        metrics.fileSize = getFileSize(downloadPath);
        
        // Calculate throughput
        double durationSec = metrics.durationMs / 1000.0;
        double sizeMB = static_cast<double>(metrics.fileSize) / (1024.0 * 1024.0);
        metrics.throughputMBps = (durationSec > 0) ? (sizeMB / durationSec) : 0.0;
        metrics.bytesReceived = metrics.fileSize;
    } else {
        metrics.errorMsg = "GET operation failed";
    }
    
    return metrics;
}

/**
 * Write CSV header
 */
void writeCSVHeader(ofstream& file) {
    file << "ClientID,Iteration,Operation,Filename,FileSizeBytes,";
    file << "StartTime,EndTime,DurationMs,";
    file << "ThroughputMBps,AvgRttMs,";
    file << "BytesSent,BytesReceived,";
    file << "Success,ErrorMsg" << endl;
}

/**
 * Write metrics to CSV
 */
void writeMetricsToCSV(ofstream& file, const ClientTestMetrics& metrics) {
    file << metrics.clientId << ","
         << metrics.iteration << ","
         << metrics.operation << ","
         << metrics.filename << ","
         << metrics.fileSize << ","
         << metrics.startTime << ","
         << metrics.endTime << ","
         << fixed << setprecision(3) << metrics.durationMs << ","
         << fixed << setprecision(3) << metrics.throughputMBps << ","
         << fixed << setprecision(3) << metrics.avgRttMs << ","
         << metrics.bytesSent << ","
         << metrics.bytesReceived << ","
         << (metrics.success ? "true" : "false") << ","
         << "\"" << metrics.errorMsg << "\"" << endl;
}

/**
 * Print metrics summary
 */
void printMetricsSummary(const vector<ClientTestMetrics>& allMetrics) {
    cout << "\n========== Test Summary ==========" << endl;
    
    int totalTests = allMetrics.size();
    int successCount = 0;
    double totalDuration = 0.0;
    double totalThroughput = 0.0;
    double totalRtt = 0.0;
    uint64_t totalBytes = 0;
    
    for (const auto& m : allMetrics) {
        if (m.success) {
            successCount++;
            totalDuration += m.durationMs;
            totalThroughput += m.throughputMBps;
            totalRtt += m.avgRttMs;
            totalBytes += m.fileSize;
        }
    }
    
    cout << "Total Tests: " << totalTests << endl;
    cout << "Successful: " << successCount << endl;
    cout << "Failed: " << (totalTests - successCount) << endl;
    
    if (successCount > 0) {
        cout << "\nAverage Metrics (Successful Tests):" << endl;
        cout << "  Duration: " << fixed << setprecision(2) 
             << (totalDuration / successCount) << " ms" << endl;
        cout << "  Throughput: " << fixed << setprecision(2) 
             << (totalThroughput / successCount) << " MB/s" << endl;
        cout << "  RTT: " << fixed << setprecision(2) 
             << (totalRtt / successCount) << " ms" << endl;
        cout << "  Total Bytes Transferred: " << totalBytes << " bytes" << endl;
    }
    
    cout << "=================================\n" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 6) {
        cerr << "Usage: " << argv[0] << " <server_ip> <port> <client_id> <num_iterations> <test_file> [output_csv]" << endl;
        cerr << "Example: " << argv[0] << " 127.0.0.1 8080 1 10 test_files/test_file.txt client_metrics.csv" << endl;
        return 1;
    }
    
    string serverIp = argv[1];
    uint16_t port = static_cast<uint16_t>(stoi(argv[2]));
    int clientId = stoi(argv[3]);
    int numIterations = stoi(argv[4]);
    string testFile = argv[5];
    string outputCsv = (argc >= 7) ? argv[6] : "concurrent_client_metrics_" + to_string(time(nullptr)) + ".csv";
    
    cout << "=== Concurrent Client Test - Scenario 1 ===" << endl;
    cout << "Server: " << serverIp << ":" << port << endl;
    cout << "Client ID: " << clientId << endl;
    cout << "Iterations: " << numIterations << endl;
    cout << "Test File: " << testFile << endl;
    cout << "Output CSV: " << outputCsv << endl;
    cout << "==========================================\n" << endl;
    
    // Verify test file exists
    if (getFileSize(testFile) == 0) {
        cerr << "Error: Test file not found or empty: " << testFile << endl;
        return 1;
    }
    
    // Open CSV file
    ofstream csvFile(outputCsv, ios::app);
    if (!csvFile.is_open()) {
        cerr << "Error: Cannot open output CSV file: " << outputCsv << endl;
        return 1;
    }
    
    // Write header if file is empty
    csvFile.seekp(0, ios::end);
    if (csvFile.tellp() == 0) {
        writeCSVHeader(csvFile);
    }
    
    vector<ClientTestMetrics> allMetrics;
    
    // Connect to server
    Client client;
    cout << "Connecting to server..." << endl;
    if (!client.connect(serverIp, port)) {
        cerr << "Error: Failed to connect to server" << endl;
        csvFile.close();
        return 1;
    }
    cout << "Connected successfully!\n" << endl;
    
    // Perform iterations
    for (int i = 1; i <= numIterations; i++) {
        cout << "Iteration " << i << "/" << numIterations << "..." << endl;
        
        // Perform PUT test
        cout << "  Uploading file..." << endl;
        ClientTestMetrics putMetrics = performPutTest(client, testFile, clientId, i);
        allMetrics.push_back(putMetrics);
        writeMetricsToCSV(csvFile, putMetrics);
        csvFile.flush();
        
        if (putMetrics.success) {
            cout << "  ✓ Upload complete: " << fixed << setprecision(2) 
                 << putMetrics.throughputMBps << " MB/s, "
                 << putMetrics.durationMs << " ms" << endl;
        } else {
            cout << "  ✗ Upload failed: " << putMetrics.errorMsg << endl;
        }
        
        // Small delay between operations
        this_thread::sleep_for(milliseconds(100));
        
        // Perform GET test
        cout << "  Downloading file..." << endl;
        string filename = extractFilename(testFile);
        ClientTestMetrics getMetrics = performGetTest(client, filename, clientId, i);
        allMetrics.push_back(getMetrics);
        writeMetricsToCSV(csvFile, getMetrics);
        csvFile.flush();
        
        if (getMetrics.success) {
            cout << "  ✓ Download complete: " << fixed << setprecision(2) 
                 << getMetrics.throughputMBps << " MB/s, "
                 << getMetrics.durationMs << " ms" << endl;
        } else {
            cout << "  ✗ Download failed: " << getMetrics.errorMsg << endl;
        }
        
        cout << endl;
        
        // Small delay between iterations
        this_thread::sleep_for(milliseconds(500));
    }
    
    // Disconnect
    client.disconnect();
    csvFile.close();
    
    // Print summary
    printMetricsSummary(allMetrics);
    
    cout << "Results saved to: " << outputCsv << endl;
    
    return 0;
}
