/**
 * File Size Test - Scenario 2
 * 
 * Tests file transfer with different file sizes to analyze the relationship
 * between file size, transfer time, and throughput.
 * 
 * Usage: ./file_size_test <server_ip> <port> <test_files_dir> [output_csv]
 * Example: ./file_size_test 127.0.0.1 8080 ./test_files file_size_metrics.csv
 */

#include "../include/client.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sys/stat.h>
#include <algorithm>
#include <cmath>
#include <dirent.h>
#include <map>
#include <thread>

using namespace std;
using namespace std::chrono;

struct FileSizeTestMetrics {
    string filename{""};
    uint64_t fileSize{0};
    string sizeCategory{""};  // "Small", "Medium", "Large"
    string operation{"PUT"};
    
    // Timing
    string timestamp{""};
    double durationMs{0.0};
    
    // Performance
    double throughputMBps{0.0};
    double avgRttMs{0.0};
    
    // Network overhead
    double connectionOverheadMs{0.0};
    double protocolOverheadPercent{0.0};
    
    // Status
    bool success{false};
    int attempt{1};
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
 * Categorize file by size
 */
string categorizeFileSize(uint64_t sizeBytes) {
    const uint64_t MB = 1024 * 1024;
    
    if (sizeBytes < MB) {
        return "Small";  // < 1 MB
    } else if (sizeBytes < 10 * MB) {
        return "Medium";  // 1-10 MB
    } else {
        return "Large";  // > 10 MB
    }
}

/**
 * Format bytes to human-readable format
 */
string formatBytes(uint64_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);
    
    while (size >= 1024.0 && unitIndex < 3) {
        size /= 1024.0;
        unitIndex++;
    }
    
    ostringstream oss;
    oss << fixed << setprecision(2) << size << " " << units[unitIndex];
    return oss.str();
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
 * Get all files in a directory recursively
 */
void getFilesRecursive(const string& dirPath, vector<string>& files) {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) return;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string name = entry->d_name;
        if (name == "." || name == "..") continue;
        
        string fullPath = dirPath;
        if (fullPath.back() != '/') fullPath += "/";
        fullPath += name;
        
        struct stat statbuf;
        if (stat(fullPath.c_str(), &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                getFilesRecursive(fullPath, files);
            } else if (S_ISREG(statbuf.st_mode)) {
                files.push_back(fullPath);
            }
        }
    }
    closedir(dir);
}

/**
 * Measure connection overhead
 */
double measureConnectionOverhead(const string& serverIp, uint16_t port) {
    auto start = high_resolution_clock::now();
    
    Client client;
    client.connect(serverIp, port);
    client.disconnect();
    
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

/**
 * Measure RTT
 */
double measureRTT(Client& client) {
    auto start = high_resolution_clock::now();
    client.listFiles();
    auto end = high_resolution_clock::now();
    return duration_cast<microseconds>(end - start).count() / 1000.0;
}

/**
 * Perform PUT test with metrics
 */
FileSizeTestMetrics performPutTest(Client& client, const string& filepath, int attempt) {
    FileSizeTestMetrics metrics;
    metrics.filename = extractFilename(filepath);
    metrics.fileSize = getFileSize(filepath);
    metrics.sizeCategory = categorizeFileSize(metrics.fileSize);
    metrics.operation = "PUT";
    metrics.attempt = attempt;
    metrics.timestamp = getCurrentTimestamp();
    
    // Measure RTT
    try {
        metrics.avgRttMs = measureRTT(client);
    } catch (...) {
        metrics.avgRttMs = 0.0;
    }
    
    // Perform PUT
    auto start = high_resolution_clock::now();
    bool result = client.putFile(filepath);
    auto end = high_resolution_clock::now();
    
    metrics.durationMs = duration_cast<microseconds>(end - start).count() / 1000.0;
    metrics.success = result;
    
    if (result) {
        // Calculate throughput
        double durationSec = metrics.durationMs / 1000.0;
        double sizeMB = static_cast<double>(metrics.fileSize) / (1024.0 * 1024.0);
        metrics.throughputMBps = (durationSec > 0) ? (sizeMB / durationSec) : 0.0;
        
        // Estimate protocol overhead (RTT + processing)
        metrics.protocolOverheadPercent = (metrics.durationMs > 0) ? 
            ((metrics.avgRttMs / metrics.durationMs) * 100.0) : 0.0;
    } else {
        metrics.errorMsg = "PUT operation failed";
    }
    
    return metrics;
}

/**
 * Perform GET test with metrics
 */
FileSizeTestMetrics performGetTest(Client& client, const string& filename, uint64_t expectedSize, int attempt) {
    FileSizeTestMetrics metrics;
    metrics.filename = filename;
    metrics.fileSize = expectedSize;
    metrics.sizeCategory = categorizeFileSize(expectedSize);
    metrics.operation = "GET";
    metrics.attempt = attempt;
    metrics.timestamp = getCurrentTimestamp();
    
    // Measure RTT
    try {
        metrics.avgRttMs = measureRTT(client);
    } catch (...) {
        metrics.avgRttMs = 0.0;
    }
    
    // Perform GET
    auto start = high_resolution_clock::now();
    bool result = client.getFile(filename);
    auto end = high_resolution_clock::now();
    
    metrics.durationMs = duration_cast<microseconds>(end - start).count() / 1000.0;
    metrics.success = result;
    
    if (result) {
        // Verify downloaded file size
        string downloadPath = "downloads/" + filename;
        uint64_t actualSize = getFileSize(downloadPath);
        
        if (actualSize != expectedSize) {
            metrics.success = false;
            metrics.errorMsg = "File size mismatch";
        } else {
            // Calculate throughput
            double durationSec = metrics.durationMs / 1000.0;
            double sizeMB = static_cast<double>(actualSize) / (1024.0 * 1024.0);
            metrics.throughputMBps = (durationSec > 0) ? (sizeMB / durationSec) : 0.0;
            
            // Estimate protocol overhead
            metrics.protocolOverheadPercent = (metrics.durationMs > 0) ? 
                ((metrics.avgRttMs / metrics.durationMs) * 100.0) : 0.0;
        }
    } else {
        metrics.errorMsg = "GET operation failed";
    }
    
    return metrics;
}

/**
 * Write CSV header
 */
void writeCSVHeader(ofstream& file) {
    file << "Filename,FileSizeBytes,SizeFormatted,SizeCategory,Operation,";
    file << "Timestamp,Attempt,DurationMs,";
    file << "ThroughputMBps,AvgRttMs,";
    file << "ConnectionOverheadMs,ProtocolOverheadPercent,";
    file << "Success,ErrorMsg" << endl;
}

/**
 * Write metrics to CSV
 */
void writeMetricsToCSV(ofstream& file, const FileSizeTestMetrics& metrics) {
    file << metrics.filename << ","
         << metrics.fileSize << ","
         << "\"" << formatBytes(metrics.fileSize) << "\","
         << metrics.sizeCategory << ","
         << metrics.operation << ","
         << metrics.timestamp << ","
         << metrics.attempt << ","
         << fixed << setprecision(3) << metrics.durationMs << ","
         << fixed << setprecision(3) << metrics.throughputMBps << ","
         << fixed << setprecision(3) << metrics.avgRttMs << ","
         << fixed << setprecision(3) << metrics.connectionOverheadMs << ","
         << fixed << setprecision(2) << metrics.protocolOverheadPercent << ","
         << (metrics.success ? "true" : "false") << ","
         << "\"" << metrics.errorMsg << "\"" << endl;
}

/**
 * Print test summary by size category
 */
void printSummary(const vector<FileSizeTestMetrics>& allMetrics) {
    cout << "\n========== Test Summary ==========" << endl;
    
    map<string, vector<FileSizeTestMetrics>> byCategory;
    for (const auto& m : allMetrics) {
        if (m.success) {
            byCategory[m.sizeCategory].push_back(m);
        }
    }
    
    for (const auto& [category, metrics] : byCategory) {
        if (metrics.empty()) continue;
        
        double avgDuration = 0.0;
        double avgThroughput = 0.0;
        double avgRtt = 0.0;
        uint64_t totalBytes = 0;
        
        for (const auto& m : metrics) {
            avgDuration += m.durationMs;
            avgThroughput += m.throughputMBps;
            avgRtt += m.avgRttMs;
            totalBytes += m.fileSize;
        }
        
        int count = metrics.size();
        avgDuration /= count;
        avgThroughput /= count;
        avgRtt /= count;
        
        cout << "\n" << category << " Files (n=" << count << "):" << endl;
        cout << "  Avg Duration: " << fixed << setprecision(2) << avgDuration << " ms" << endl;
        cout << "  Avg Throughput: " << fixed << setprecision(2) << avgThroughput << " MB/s" << endl;
        cout << "  Avg RTT: " << fixed << setprecision(2) << avgRtt << " ms" << endl;
        cout << "  Total Transferred: " << formatBytes(totalBytes) << endl;
    }
    
    cout << "\n=================================\n" << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cerr << "Usage: " << argv[0] << " <server_ip> <port> <test_files_dir> [output_csv] [num_attempts]" << endl;
        cerr << "Example: " << argv[0] << " 127.0.0.1 8080 ./test_files file_size_metrics.csv 3" << endl;
        return 1;
    }
    
    string serverIp = argv[1];
    uint16_t port = static_cast<uint16_t>(stoi(argv[2]));
    string testDir = argv[3];
    string outputCsv = (argc >= 5) ? argv[4] : "file_size_metrics_" + to_string(time(nullptr)) + ".csv";
    int numAttempts = (argc >= 6) ? stoi(argv[5]) : 3;
    
    cout << "=== File Size Test - Scenario 2 ===" << endl;
    cout << "Server: " << serverIp << ":" << port << endl;
    cout << "Test Directory: " << testDir << endl;
    cout << "Output CSV: " << outputCsv << endl;
    cout << "Attempts per file: " << numAttempts << endl;
    cout << "===================================\n" << endl;
    
    // Get all test files
    vector<string> testFiles;
    getFilesRecursive(testDir, testFiles);
    
    if (testFiles.empty()) {
        cerr << "Error: No files found in directory: " << testDir << endl;
        return 1;
    }
    
    // Sort files by size
    sort(testFiles.begin(), testFiles.end(), [](const string& a, const string& b) {
        return getFileSize(a) < getFileSize(b);
    });
    
    cout << "Found " << testFiles.size() << " test files\n" << endl;
    
    // Open CSV file
    ofstream csvFile(outputCsv);
    if (!csvFile.is_open()) {
        cerr << "Error: Cannot open output CSV file: " << outputCsv << endl;
        return 1;
    }
    writeCSVHeader(csvFile);
    
    vector<FileSizeTestMetrics> allMetrics;
    
    // Measure connection overhead
    cout << "Measuring connection overhead..." << endl;
    double connectionOverhead = measureConnectionOverhead(serverIp, port);
    cout << "Connection overhead: " << fixed << setprecision(2) << connectionOverhead << " ms\n" << endl;
    
    // Connect to server
    Client client;
    cout << "Connecting to server..." << endl;
    if (!client.connect(serverIp, port)) {
        cerr << "Error: Failed to connect to server" << endl;
        csvFile.close();
        return 1;
    }
    cout << "Connected successfully!\n" << endl;
    
    // Test each file
    for (size_t i = 0; i < testFiles.size(); i++) {
        const string& filepath = testFiles[i];
        string filename = extractFilename(filepath);
        uint64_t fileSize = getFileSize(filepath);
        
        cout << "\n[" << (i + 1) << "/" << testFiles.size() << "] Testing: " 
             << filename << " (" << formatBytes(fileSize) << ")" << endl;
        
        // Perform multiple attempts for each file
        for (int attempt = 1; attempt <= numAttempts; attempt++) {
            cout << "  Attempt " << attempt << "/" << numAttempts << endl;
            
            // PUT test
            cout << "    Uploading..." << endl;
            FileSizeTestMetrics putMetrics = performPutTest(client, filepath, attempt);
            putMetrics.connectionOverheadMs = connectionOverhead;
            allMetrics.push_back(putMetrics);
            writeMetricsToCSV(csvFile, putMetrics);
            csvFile.flush();
            
            if (putMetrics.success) {
                cout << "    ✓ Upload: " << fixed << setprecision(2) 
                     << putMetrics.throughputMBps << " MB/s" << endl;
            } else {
                cout << "    ✗ Upload failed" << endl;
                continue;
            }
            
            // Small delay
            this_thread::sleep_for(milliseconds(200));
            
            // GET test
            cout << "    Downloading..." << endl;
            FileSizeTestMetrics getMetrics = performGetTest(client, filename, fileSize, attempt);
            getMetrics.connectionOverheadMs = connectionOverhead;
            allMetrics.push_back(getMetrics);
            writeMetricsToCSV(csvFile, getMetrics);
            csvFile.flush();
            
            if (getMetrics.success) {
                cout << "    ✓ Download: " << fixed << setprecision(2) 
                     << getMetrics.throughputMBps << " MB/s" << endl;
            } else {
                cout << "    ✗ Download failed" << endl;
            }
            
            // Delay between attempts
            if (attempt < numAttempts) {
                this_thread::sleep_for(milliseconds(500));
            }
        }
    }
    
    // Disconnect
    client.disconnect();
    csvFile.close();
    
    // Print summary
    printSummary(allMetrics);
    
    cout << "Results saved to: " << outputCsv << endl;
    
    return 0;
}
