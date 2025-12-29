/**
 * Bulk Upload Client - Upload all files in a directory to server
 * 
 * Usage: ./bulk_upload_client <server_ip> <port> <directory_path>
 * Example: ./bulk_upload_client 127.0.0.1 8080 ./test_files
 */

#include "../include/client.h"
#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <chrono>
#include <iomanip>

using namespace std;
using namespace std::chrono;

struct UploadStats {
    int totalFiles{0};
    int successFiles{0};
    int failedFiles{0};
    uint64_t totalBytes{0};
    double totalTime{0.0};
    vector<string> failedFileNames;
};

/**
 * Get all files in a directory (non-recursive)
 */
vector<string> getFilesInDirectory(const string& dirPath) {
    vector<string> files;
    DIR* dir = opendir(dirPath.c_str());
    
    if (!dir) {
        cerr << "Error: Cannot open directory: " << dirPath << endl;
        return files;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        string filename = entry->d_name;
        
        // Skip . and ..
        if (filename == "." || filename == "..") {
            continue;
        }
        
        // Construct full path
        string fullPath = dirPath;
        if (fullPath.back() != '/') {
            fullPath += "/";
        }
        fullPath += filename;
        
        // Check if it's a regular file
        struct stat statbuf;
        if (stat(fullPath.c_str(), &statbuf) == 0) {
            if (S_ISREG(statbuf.st_mode)) {
                files.push_back(fullPath);
            }
        }
    }
    
    closedir(dir);
    return files;
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
 * Extract filename from full path
 */
string extractFilename(const string& filepath) {
    size_t pos = filepath.find_last_of("/\\");
    if (pos != string::npos) {
        return filepath.substr(pos + 1);
    }
    return filepath;
}

/**
 * Display progress bar
 */
void displayProgress(int current, int total, const string& currentFile) {
    int barWidth = 40;
    float progress = static_cast<float>(current) / total;
    int pos = static_cast<int>(barWidth * progress);
    
    cout << "\r[";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) cout << "=";
        else if (i == pos) cout << ">";
        else cout << " ";
    }
    cout << "] " << static_cast<int>(progress * 100.0) << "% ("
         << current << "/" << total << ") " 
         << currentFile << "        " << flush;
}

/**
 * Upload all files in directory
 */
void bulkUpload(Client& client, const string& dirPath, UploadStats& stats) {
    cout << "\n=== Bulk Upload Client ===\n" << endl;
    cout << "Scanning directory: " << dirPath << endl;
    
    // Get all files
    vector<string> files = getFilesInDirectory(dirPath);
    
    if (files.empty()) {
        cout << "No files found in directory!" << endl;
        return;
    }
    
    stats.totalFiles = files.size();
    cout << "Found " << stats.totalFiles << " files\n" << endl;
    
    // Calculate total size
    uint64_t totalSize = 0;
    for (const auto& file : files) {
        totalSize += getFileSize(file);
    }
    cout << "Total size: " << formatBytes(totalSize) << "\n" << endl;
    
    // Confirm upload
    cout << "Start upload? (y/n): ";
    string confirm;
    getline(cin, confirm);
    if (confirm != "y" && confirm != "Y") {
        cout << "Upload cancelled." << endl;
        return;
    }
    
    cout << "\nUploading files...\n" << endl;
    
    // Start timing
    auto startTime = high_resolution_clock::now();
    
    // Upload each file
    for (size_t i = 0; i < files.size(); ++i) {
        const string& filepath = files[i];
        string filename = extractFilename(filepath);
        uint64_t fileSize = getFileSize(filepath);
        
        // Display progress
        displayProgress(i, files.size(), filename);
        
        // Upload file
        bool success = client.putFile(filepath);
        
        if (success) {
            stats.successFiles++;
            stats.totalBytes += fileSize;
        } else {
            stats.failedFiles++;
            stats.failedFileNames.push_back(filename);
        }
    }
    
    // Display final progress
    displayProgress(files.size(), files.size(), "Complete!");
    cout << endl;
    
    // End timing
    auto endTime = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(endTime - startTime);
    stats.totalTime = duration.count() / 1000.0; // Convert to ms
}

/**
 * Display upload statistics
 */
void displayStats(const UploadStats& stats) {
    cout << "\n=== Upload Summary ===" << endl;
    cout << "Total files:    " << stats.totalFiles << endl;
    cout << "Successful:     " << stats.successFiles << " (" 
         << (stats.totalFiles > 0 ? (stats.successFiles * 100 / stats.totalFiles) : 0) 
         << "%)" << endl;
    cout << "Failed:         " << stats.failedFiles << endl;
    cout << "Total size:     " << formatBytes(stats.totalBytes) << endl;
    
    // Display time in appropriate unit
    if (stats.totalTime < 1000.0) {
        cout << "Total time:     " << fixed << setprecision(2) 
             << stats.totalTime << " ms" << endl;
    } else {
        cout << "Total time:     " << fixed << setprecision(3) 
             << stats.totalTime / 1000.0 << " seconds" << endl;
    }
    
    if (stats.totalTime > 0) {
        double throughputKbps = (stats.totalBytes * 8.0) / stats.totalTime / 1024.0;
        cout << "Avg throughput: " << fixed << setprecision(2) 
             << throughputKbps << " kbps" << endl;
    }
    
    if (!stats.failedFileNames.empty()) {
        cout << "\nFailed files:" << endl;
        for (const auto& filename : stats.failedFileNames) {
            cout << "  - " << filename << endl;
        }
    }
    
    cout << "\n=== Client Metrics ===" << endl;
}

int main(int argc, char* argv[]) {
    // Check arguments
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <server_ip> <port> <directory_path>" << endl;
        cerr << "Example: " << argv[0] << " 127.0.0.1 8080 ./test_files" << endl;
        return 1;
    }
    
    string serverIp = argv[1];
    uint16_t port = static_cast<uint16_t>(stoi(argv[2]));
    string dirPath = argv[3];
    
    // Create client
    Client client;
    client.setVerbose(false); // Disable verbose for cleaner output
    
    // Connect to server
    cout << "Connecting to " << serverIp << ":" << port << "..." << endl;
    if (!client.connect(serverIp, port)) {
        cerr << "Failed to connect to server!" << endl;
        return 1;
    }
    cout << "Connected successfully!\n" << endl;
    
    // Upload files
    UploadStats stats;
    bulkUpload(client, dirPath, stats);
    
    // Display statistics
    displayStats(stats);
    client.displayMetrics();
    
    // Offer to view request history
    cout << "\nView request history? (y/n): ";
    string historyChoice;
    getline(cin, historyChoice);
    if (historyChoice == "y" || historyChoice == "Y") {
        cout << "\nHow many recent requests to show? (0 = all, default = 20): ";
        string limitStr;
        getline(cin, limitStr);
        size_t limit = limitStr.empty() ? 20 : stoul(limitStr);
        client.displayHistory(limit);
    }
    
    // Offer to export metrics
    cout << "\nExport metrics to CSV? (y/n): ";
    string exportChoice;
    getline(cin, exportChoice);
    if (exportChoice == "y" || exportChoice == "Y") {
        string filename = "bulk_upload_metrics_" + 
                         to_string(time(nullptr)) + ".csv";
        client.exportMetrics(filename);
        cout << "Metrics exported to: " << filename << endl;
    }
    
    // Disconnect
    client.disconnect();
    cout << "\nDisconnected from server." << endl;
    
    return (stats.failedFiles == 0) ? 0 : 1;
}
