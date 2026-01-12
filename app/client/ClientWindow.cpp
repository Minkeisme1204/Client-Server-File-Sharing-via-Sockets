#include "ClientWindow.h"
#include "../common/NetworkUtils.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QFileInfo>
#include <QDateTime>
#include <QGridLayout>
#include <QSplitter>
#include <QTimer>
#include <sstream>
#include <iostream>

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent), client(std::make_unique<Client>()), currentPort(0), wasConnected(false) {
    setupUI();
    
    // Setup status update timer
    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &ClientWindow::updateStatus);
    statusTimer->start(500); // Update every 500ms for faster real-time updates
}

ClientWindow::~ClientWindow() {
    if (client && client->isConnected()) {
        client->disconnect();
    }
}

void ClientWindow::setupUI() {
    setWindowTitle("File Transfer Client");
    setMinimumSize(560, 600);
    
    // Apply light theme styling similar to the image
    setStyleSheet(R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        QLabel {
            color: #333;
            font-size: 11pt;
        }
        QLabel#statusBar {
            background-color: #4CAF50;
            color: white;
            padding: 8px;
            font-weight: bold;
        }
        QLabel#statusBarDisconnected {
            background-color: #f44336;
            color: white;
            padding: 8px;
            font-weight: bold;
        }
        QComboBox {
            background-color: white;
            border: 1px solid #ccc;
            padding: 5px;
            font-size: 10pt;
        }
        QPushButton {
            background-color: #2196F3;
            color: white;
            border: none;
            padding: 8px 16px;
            font-weight: bold;
            border-radius: 3px;
            min-height: 30px;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
        QPushButton:pressed {
            background-color: #0D47A1;
        }
        QPushButton:disabled {
            background-color: #ccc;
            color: #888;
        }
        QPushButton#disconnectButton {
            background-color: #FF5722;
        }
        QPushButton#disconnectButton:hover {
            background-color: #E64A19;
        }
        QPushButton#quitButton {
            background-color: #9E9E9E;
        }
        QPushButton#quitButton:hover {
            background-color: #757575;
        }
        QTabWidget::pane {
            border: 1px solid #ccc;
            background-color: white;
        }
        QTabBar::tab {
            background-color: #424a5d;
            color: white;
            padding: 8px 16px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background-color: #546e7a;
        }
        QListWidget {
            background-color: white;
            border: 1px solid #ccc;
            font-size: 10pt;
        }
        QListWidget::item {
            padding: 5px;
        }
        QListWidget::item:selected {
            background-color: #2196F3;
            color: white;
        }
        QTextEdit {
            background-color: white;
            border: 1px solid #ccc;
            font-family: Consolas, monospace;
            font-size: 9pt;
        }
        QGroupBox {
            border: 1px solid #ccc;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
            background-color: white;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
    )");
    
    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    
    // Status bar at top
    statusLabel = new QLabel("Status: Disconnected", this);
    statusLabel->setObjectName("statusBarDisconnected");
    mainLayout->addWidget(statusLabel);
    
    serverInfoLabel = new QLabel("Not connected to server", this);
    serverInfoLabel->setStyleSheet("QLabel { color: #666; padding: 5px; background-color: #e3f2fd; }");
    mainLayout->addWidget(serverInfoLabel);
    
    uptimeLabel = new QLabel("Connection time: --:--:--", this);
    uptimeLabel->setStyleSheet("QLabel { color: #666; padding: 5px; background-color: #fff3e0; font-weight: bold; }");
    mainLayout->addWidget(uptimeLabel);
    
    connectionLabel = new QLabel("Not connected to server", this);
    connectionLabel->setStyleSheet("QLabel { color: #666; padding: 5px; }");
    mainLayout->addWidget(connectionLabel);
    
    // Command section
    QHBoxLayout* commandLayout = new QHBoxLayout();
    QLabel* cmdLabel = new QLabel("Command:", this);
    commandLayout->addWidget(cmdLabel);
    
    commandCombo = new QComboBox(this);
    commandCombo->setEditable(true);
    
    // Auto-detect Tailscale IP for default command
    QString tailscaleIP = NetworkUtils::getTailscaleIP();
    if (!tailscaleIP.isEmpty()) {
        commandCombo->addItem(QString("connect %1 9000").arg(tailscaleIP));
    } else {
        commandCombo->addItem("connect 192.168.1.100 9000");
    }
    
    commandCombo->addItem("list");
    commandCombo->addItem("get <filename>");
    commandCombo->addItem("put <filepath>");
    commandCombo->addItem("metrics");
    commandCombo->addItem("disconnect");
    commandCombo->setCurrentIndex(0);
    commandLayout->addWidget(commandCombo, 1);
    
    executeButton = new QPushButton("Connect", this);
    connect(executeButton, &QPushButton::clicked, this, &ClientWindow::onExecuteCommand);
    commandLayout->addWidget(executeButton);
    
    mainLayout->addLayout(commandLayout);
    
    // Tab widget for Server Files and Transfer Log
    tabWidget = new QTabWidget(this);
    
    // Server Files tab
    fileListWidget = new QListWidget(this);
    // Add placeholder message
    QListWidgetItem* placeholderItem = new QListWidgetItem("💡 Click 'List Files' button to view files on server");
    placeholderItem->setFlags(placeholderItem->flags() & ~Qt::ItemIsSelectable);
    placeholderItem->setForeground(QColor("#999"));
    fileListWidget->addItem(placeholderItem);
    tabWidget->addTab(fileListWidget, "Server Files");
    
    // Transfer Log tab
    transferLogText = new QTextEdit(this);
    transferLogText->setReadOnly(true);
    tabWidget->addTab(transferLogText, "Transfer Log");
    
    mainLayout->addWidget(tabWidget, 1);
    
    // Action buttons row 1
    QHBoxLayout* buttonsRow1 = new QHBoxLayout();
    
    // Quick Connect button with Tailscale
    QPushButton* quickConnectButton = new QPushButton("🌐 Quick Connect (Tailscale)", this);
    quickConnectButton->setStyleSheet("QPushButton { background-color: #4CAF50; }");
    connect(quickConnectButton, &QPushButton::clicked, this, [this]() {
        QString tailscaleIP = NetworkUtils::getTailscaleIP();
        if (tailscaleIP.isEmpty()) {
            QMessageBox::warning(this, "Tailscale Not Found", 
                "Tailscale IP not detected. Please:\n\n"
                "1. Install Tailscale: curl -fsSL https://tailscale.com/install.sh | sh\n"
                "2. Login: sudo tailscale up\n"
                "3. Check status: tailscale status\n\n"
                "Or use regular Connect button for local network.");
            return;
        }
        
        bool ok;
        int port = QInputDialog::getInt(this, "Quick Connect",
            QString("Connect to Tailscale IP: %1\n\nEnter port:").arg(tailscaleIP),
            9000, 1, 65535, 1, &ok);
        
        if (ok) {
            connectToServer(tailscaleIP, port);
        }
    });
    buttonsRow1->addWidget(quickConnectButton);
    
    disconnectButton = new QPushButton("Disconnect", this);
    disconnectButton->setObjectName("disconnectButton");
    disconnectButton->setEnabled(false);
    connect(disconnectButton, &QPushButton::clicked, this, &ClientWindow::onDisconnectClicked);
    buttonsRow1->addWidget(disconnectButton);
    
    listFilesButton = new QPushButton("List Files", this);
    listFilesButton->setEnabled(false);
    connect(listFilesButton, &QPushButton::clicked, this, &ClientWindow::onListFilesClicked);
    buttonsRow1->addWidget(listFilesButton);
    
    downloadButton = new QPushButton("Download File", this);
    downloadButton->setEnabled(false);
    connect(downloadButton, &QPushButton::clicked, this, &ClientWindow::onDownloadClicked);
    buttonsRow1->addWidget(downloadButton);
    
    mainLayout->addLayout(buttonsRow1);
    
    // Action buttons row 2
    QHBoxLayout* buttonsRow2 = new QHBoxLayout();
    
    uploadButton = new QPushButton("Upload File", this);
    uploadButton->setEnabled(false);
    connect(uploadButton, &QPushButton::clicked, this, &ClientWindow::onUploadClicked);
    buttonsRow2->addWidget(uploadButton);
    
    metricsButton = new QPushButton("Reset Metrics", this);
    metricsButton->setEnabled(false);
    connect(metricsButton, &QPushButton::clicked, this, &ClientWindow::onResetMetricsClicked);
    buttonsRow2->addWidget(metricsButton);
    
    exportButton = new QPushButton("Export CSV", this);
    exportButton->setEnabled(false);
    connect(exportButton, &QPushButton::clicked, this, &ClientWindow::onExportClicked);
    buttonsRow2->addWidget(exportButton);
    
    mainLayout->addLayout(buttonsRow2);
    
    // Client Metrics section
    QGroupBox* metricsGroup = new QGroupBox("Client Metrics", this);
    QGridLayout* metricsLayout = new QGridLayout(metricsGroup);
    
    metricsLayout->addWidget(new QLabel("Bytes Sent:", this), 0, 0);
    bytesSentLabel = new QLabel("0 MB", this);
    bytesSentLabel->setStyleSheet("QLabel { font-weight: normal; }");
    metricsLayout->addWidget(bytesSentLabel, 0, 1);
    
    metricsLayout->addWidget(new QLabel("Bytes Received:", this), 0, 2);
    bytesReceivedLabel = new QLabel("0 MB", this);
    bytesReceivedLabel->setStyleSheet("QLabel { font-weight: normal; }");
    metricsLayout->addWidget(bytesReceivedLabel, 0, 3);
    
    metricsLayout->addWidget(new QLabel("Files Uploaded:", this), 1, 0);
    filesUploadedLabel = new QLabel("0", this);
    filesUploadedLabel->setStyleSheet("QLabel { font-weight: normal; }");
    metricsLayout->addWidget(filesUploadedLabel, 1, 1);
    
    metricsLayout->addWidget(new QLabel("Files Downloaded:", this), 1, 2);
    filesDownloadedLabel = new QLabel("0", this);
    filesDownloadedLabel->setStyleSheet("QLabel { font-weight: normal; }");
    metricsLayout->addWidget(filesDownloadedLabel, 1, 3);
    
    metricsLayout->addWidget(new QLabel("Avg Throughput (Sent):", this), 2, 0);
    avgThroughputSentLabel = new QLabel("0.00 Mbps", this);
    avgThroughputSentLabel->setStyleSheet("QLabel { font-weight: normal; }");
    metricsLayout->addWidget(avgThroughputSentLabel, 2, 1);
    
    metricsLayout->addWidget(new QLabel("Avg Throughput (Recv):", this), 2, 2);
    avgThroughputRecvLabel = new QLabel("0.00 Mbps", this);
    avgThroughputRecvLabel->setStyleSheet("QLabel { font-weight: normal; }");
    metricsLayout->addWidget(avgThroughputRecvLabel, 2, 3);
    
    metricsLayout->addWidget(new QLabel("Packet Loss (Download):", this), 3, 0);
    packetLossLabel = new QLabel("0.00% (0/0)", this);
    packetLossLabel->setStyleSheet("QLabel { font-weight: normal; }");
    metricsLayout->addWidget(packetLossLabel, 3, 1, 1, 3);
    
    mainLayout->addWidget(metricsGroup);
    
    // Quit button
    quitButton = new QPushButton("Quit", this);
    quitButton->setObjectName("quitButton");
    connect(quitButton, &QPushButton::clicked, this, &ClientWindow::onQuitClicked);
    mainLayout->addWidget(quitButton);
    
    setCentralWidget(centralWidget);
    
    appendLog("Client initialized. Ready to connect.", true);
    updateMetrics();
}

void ClientWindow::appendLog(const QString& text, bool success) {
    QString color = success ? "green" : "red";
    QString icon = success ? "✓" : "✗";
    QString html = QString("<span style='color: %1;'>%2 %3</span><br>")
                       .arg(color)
                       .arg(icon)
                       .arg(text.toHtmlEscaped());
    transferLogText->moveCursor(QTextCursor::End);
    transferLogText->insertHtml(html);
    transferLogText->moveCursor(QTextCursor::End);
}

void ClientWindow::connectToServer(const QString& ip, int port) {
    if (client->isConnected()) {
        appendLog("Already connected. Disconnect first.", false);
        QMessageBox::warning(this, "Already Connected", "You are already connected to a server.\nDisconnect first before connecting to another server.");
        return;
    }
    
    appendLog(QString("Connecting to %1:%2...").arg(ip).arg(port), true);
    
    if (client->connect(ip.toStdString(), static_cast<uint16_t>(port))) {
        // Connection successful - verify by testing a simple operation
        // Try to get file list to confirm server is actually responding
        std::vector<std::string> testList = client->getFileList();
        
        // Check if we got a valid response
        if (testList.empty() && !client->isConnected()) {
            // Connection dropped immediately - server not running properly
            appendLog("Connection failed: Server not responding", false);
            QMessageBox::critical(this, "Connection Failed", 
                QString("Failed to connect to %1:%2\n\nPossible reasons:\n"
                        "• Server is not running\n"
                        "• Server rejected the connection\n"
                        "• Network issue").arg(ip).arg(port));
            return;
        }
        
        currentIP = ip;
        currentPort = port;
        connectionStartTime = QDateTime::currentDateTime();
        appendLog(QString("✓ Successfully connected to %1:%2").arg(ip).arg(port), true);
        appendLog("Use 'List Files' button to view available files", true);
        QMessageBox::information(this, "Connected", 
            QString("Successfully connected to server\n%1:%2").arg(ip).arg(port));
    } else {
        appendLog("✗ Connection failed: Unable to reach server", false);
        QMessageBox::critical(this, "Connection Failed", 
            QString("Failed to connect to %1:%2\n\nPossible reasons:\n"
                    "• Server is not running\n"
                    "• Incorrect IP address or port\n"
                    "• Firewall blocking connection\n"
                    "• Network issue").arg(ip).arg(port));
    }
}

void ClientWindow::onExecuteCommand() {
    QString command = commandCombo->currentText().trimmed();
    if (command.isEmpty()) return;
    
    processCommand(command);
}

void ClientWindow::processCommand(const QString& command) {
    QStringList parts = command.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) return;
    
    QString cmd = parts[0].toLower();
    
    if (cmd == "connect") {
        if (parts.size() < 3) {
            appendLog("Usage: connect <ip> <port>", false);
            return;
        }
        QString ip = parts[1];
        bool ok;
        int port = parts[2].toInt(&ok);
        if (!ok || port <= 0 || port > 65535) {
            appendLog("Invalid port number", false);
            return;
        }
        connectToServer(ip, port);
    }
    else if (cmd == "disconnect") {
        onDisconnectClicked();
    }
    else if (cmd == "list") {
        onListFilesClicked();
    }
    else if (cmd == "get") {
        if (parts.size() < 2) {
            appendLog("Usage: get <filename>", false);
            return;
        }
        if (!client->isConnected()) {
            appendLog("Not connected to server", false);
            return;
        }
        // Get filename (may contain spaces) - everything after "get "
        QString filename = command.mid(4).trimmed(); // Remove "get " prefix
        QString saveDir = QFileDialog::getExistingDirectory(this, "Select Download Directory", ".");
        if (!saveDir.isEmpty()) {
            appendLog(QString("Downloading %1...").arg(filename), true);
            if (client->getFile(filename.toStdString(), saveDir.toStdString())) {
                appendLog(QString("Download completed: %1").arg(filename), true);
            } else {
                appendLog(QString("Download failed: %1").arg(filename), false);
            }
        }
    }
    else if (cmd == "put") {
        if (parts.size() < 2) {
            appendLog("Usage: put <filepath>", false);
            return;
        }
        if (!client->isConnected()) {
            appendLog("Not connected to server", false);
            return;
        }
        // Get filepath (may contain spaces) - everything after "put "
        QString filepath = command.mid(4).trimmed(); // Remove "put " prefix
        if (client->putFile(filepath.toStdString())) {
            appendLog(QString("Uploaded: %1").arg(filepath), true);
        } else {
            appendLog(QString("Upload failed: %1").arg(filepath), false);
        }
    }
    else if (cmd == "reset") {
        onResetMetricsClicked();
    }
    else {
        appendLog("Unknown command: " + cmd, false);
    }
}

void ClientWindow::onConnectClicked() {
    // Auto-detect Tailscale IP for default value
    QString tailscaleIP = NetworkUtils::getTailscaleIP();
    QString defaultValue;
    
    if (!tailscaleIP.isEmpty()) {
        defaultValue = QString("%1:9000").arg(tailscaleIP);
    } else {
        QString localIP = NetworkUtils::getLocalIP();
        defaultValue = QString("%1:9000").arg(localIP);
    }
    
    // Build prompt message
    QString promptMsg = "Enter IP:Port (e.g., 192.168.1.100:9000):";
    if (!tailscaleIP.isEmpty()) {
        promptMsg = QString("🌐 Tailscale IP detected: %1\n\nEnter IP:Port:").arg(tailscaleIP);
    }
    
    bool ok;
    QString input = QInputDialog::getText(this, "Connect to Server",
                                         promptMsg,
                                         QLineEdit::Normal, defaultValue, &ok);
    if (ok && !input.isEmpty()) {
        QStringList parts = input.split(':');
        if (parts.size() == 2) {
            QString ip = parts[0];
            bool portOk;
            int port = parts[1].toInt(&portOk);
            if (portOk) {
                connectToServer(ip, port);
            }
        }
    }
}

void ClientWindow::onDisconnectClicked() {
    if (client->isConnected()) {
        client->disconnect();
        appendLog("Disconnected from server", true);
        fileListWidget->clear();
    }
}

void ClientWindow::onListFilesClicked() {
    if (!client->isConnected()) {
        QMessageBox::warning(this, "Not Connected", "Please connect to server first");
        return;
    }
    appendLog("Requesting file list from server...", true);
    
    // Get actual file list from server
    std::vector<std::string> fileList = client->getFileList();
    
    // Update the file list widget
    fileListWidget->clear();
    
    if (fileList.empty()) {
        appendLog("No files available on server", true);
    } else {
        for (const auto& filename : fileList) {
            QString qFilename = QString::fromStdString(filename);
            
            // Add emoji based on file extension
            QString displayName;
            if (qFilename.endsWith(".pdf")) {
                displayName = "📄 " + qFilename;
            } else if (qFilename.endsWith(".png") || qFilename.endsWith(".jpg") || qFilename.endsWith(".jpeg")) {
                displayName = "🖼 " + qFilename;
            } else if (qFilename.endsWith(".csv") || qFilename.endsWith(".xlsx")) {
                displayName = "📊 " + qFilename;
            } else if (qFilename.endsWith(".txt")) {
                displayName = "📝 " + qFilename;
            } else if (qFilename.endsWith(".zip") || qFilename.endsWith(".tar") || qFilename.endsWith(".gz")) {
                displayName = "📦 " + qFilename;
            } else {
                displayName = "📄 " + qFilename;
            }
            
            fileListWidget->addItem(displayName);
        }
        appendLog(QString("Received %1 file(s) from server").arg(fileList.size()), true);
    }
}

// refreshFileList() removed - now using getFileList() which returns actual server files

void ClientWindow::onDownloadClicked() {
    if (!client->isConnected()) {
        QMessageBox::warning(this, "Not Connected", "Please connect to server first");
        return;
    }
    
    QListWidgetItem* item = fileListWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "No File Selected", "Please select a file to download");
        return;
    }
    
    QString filename = item->text();
    // Remove emoji prefix if exists - check for all possible emoji types
    if (filename.contains(" ") && (
        filename.startsWith("📄 ") || 
        filename.startsWith("📁 ") || 
        filename.startsWith("🖼 ") || 
        filename.startsWith("📊 ") || 
        filename.startsWith("📝 ") || 
        filename.startsWith("📦 "))) {
        // Find first space and take everything after it
        int spaceIndex = filename.indexOf(" ");
        if (spaceIndex > 0) {
            filename = filename.mid(spaceIndex + 1).trimmed();
        }
    }
    
    QString saveDir = QFileDialog::getExistingDirectory(this, "Select Download Directory", ".");
    if (!saveDir.isEmpty()) {
        appendLog(QString("Downloading %1...").arg(filename), true);
        if (client->getFile(filename.toStdString(), saveDir.toStdString())) {
            appendLog(QString("Download completed: %1").arg(filename), true);
        } else {
            appendLog(QString("Download failed: %1").arg(filename), false);
        }
    }
}

void ClientWindow::onUploadClicked() {
    if (!client->isConnected()) {
        QMessageBox::warning(this, "Not Connected", "Please connect to server first");
        return;
    }
    
    QString filepath = QFileDialog::getOpenFileName(this, "Select File to Upload");
    if (!filepath.isEmpty()) {
        QFileInfo fileInfo(filepath);
        appendLog(QString("Uploading %1...").arg(fileInfo.fileName()), true);
        if (client->putFile(filepath.toStdString())) {
            appendLog(QString("Upload completed: %1").arg(fileInfo.fileName()), true);
        } else {
            appendLog("Upload failed", false);
        }
    }
}

void ClientWindow::onResetMetricsClicked() {
    client->resetMetrics();
    updateMetrics();
    appendLog("Metrics have been reset", true);
}

void ClientWindow::onExportClicked() {
    QString filename = QFileDialog::getSaveFileName(this, "Export Metrics",
        QString("client_metrics_%1.csv").arg(QDateTime::currentSecsSinceEpoch()),
        "CSV Files (*.csv)");
    
    if (!filename.isEmpty()) {
        if (client->exportMetrics(filename.toStdString())) {
            appendLog(QString("Metrics exported to: %1").arg(filename), true);
            QMessageBox::information(this, "Success", "Metrics exported successfully!");
        } else {
            appendLog("Failed to export metrics", false);
        }
    }
}

void ClientWindow::onQuitClicked() {
    if (client->isConnected()) {
        client->disconnect();
    }
    close();
}

void ClientWindow::updateStatus() {
    updateStatusBar();
    updateMetrics();
}

void ClientWindow::updateStatusBar() {
    bool currentlyConnected = client->isConnected();
    
    // Detect server disconnect
    if (wasConnected && !currentlyConnected) {
        // Connection was lost
        appendLog("✗ Connection lost: Server disconnected", false);
        QMessageBox::warning(this, "Connection Lost", 
            QString("Lost connection to server %1:%2\n\nThe server may have stopped or network issue occurred.")
            .arg(currentIP).arg(currentPort));
        currentIP.clear();
        currentPort = 0;
    }
    
    wasConnected = currentlyConnected;
    
    if (currentlyConnected) {
        statusLabel->setText(QString("Status: Running"));
        statusLabel->setObjectName("statusBar");
        statusLabel->setStyleSheet("QLabel { background-color: #4CAF50; color: white; padding: 8px; font-weight: bold; }");
        
        // Calculate connection time
        qint64 connTime = connectionStartTime.secsTo(QDateTime::currentDateTime());
        int hours = connTime / 3600;
        int minutes = (connTime % 3600) / 60;
        int seconds = connTime % 60;
        
        QString timeStr = QString("Connection time: %1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
        uptimeLabel->setText(timeStr);
        uptimeLabel->setStyleSheet("QLabel { color: #4CAF50; padding: 5px; background-color: #e8f5e9; font-weight: bold; }");
        
        serverInfoLabel->setText(QString("Connected to: %1:%2").arg(currentIP).arg(currentPort));
        connectionLabel->setText(QString("Connected | Ready to transfer files"));
        
        disconnectButton->setEnabled(true);
        listFilesButton->setEnabled(true);
        downloadButton->setEnabled(true);
        uploadButton->setEnabled(true);
        metricsButton->setEnabled(true);
        exportButton->setEnabled(true);
        executeButton->setText("Execute");
    } else {
        statusLabel->setText("Status: Disconnected");
        statusLabel->setObjectName("statusBarDisconnected");
        statusLabel->setStyleSheet("QLabel { background-color: #f44336; color: white; padding: 8px; font-weight: bold; }");
        
        uptimeLabel->setText("Connection time: --:--:--");
        uptimeLabel->setStyleSheet("QLabel { color: #666; padding: 5px; background-color: #fff3e0; font-weight: bold; }");
        
        serverInfoLabel->setText("Not connected to server");
        connectionLabel->setText("Not connected to server");
        
        disconnectButton->setEnabled(false);
        listFilesButton->setEnabled(false);
        downloadButton->setEnabled(false);
        uploadButton->setEnabled(false);
        metricsButton->setEnabled(false);
        exportButton->setEnabled(false);
        executeButton->setText("Connect");
    }
}

void ClientWindow::updateMetrics() {
    const ClientMetrics& metrics = client->getMetrics();
    
    double bytesSent = metrics.total_bytes_sent.load();
    double bytesReceived = metrics.total_bytes_received.load();
    
    bytesSentLabel->setText(QString("%1 KB").arg(bytesSent / 1024.0, 0, 'f', 2));
    bytesReceivedLabel->setText(QString("%1 KB").arg(bytesReceived / 1024.0, 0, 'f', 2));
    
    // Count uploads/downloads from history
    int uploads = 0, downloads = 0;
    for (const auto& record : metrics.request_history) {
        if (record.operation == "PUT") uploads++;
        if (record.operation == "GET") downloads++;
    }
    
    filesUploadedLabel->setText(QString::number(uploads));
    filesDownloadedLabel->setText(QString::number(downloads));
    
    // Calculate separate throughput for sent and received in Mbps
    uint64_t total_time = metrics.total_transfer_time_ms.load();
    double throughputSentMbps = 0.0;
    double throughputRecvMbps = 0.0;
    
    std::cout << "[DEBUG] Total time: " << total_time << " ms" << std::endl;
    
    if (total_time > 0) {
        // Calculate upload throughput: (bytes * 8) / (time_ms * 1000) = Mbps
        throughputSentMbps = (bytesSent * 8.0) / (total_time * 1000.0);
        // Calculate download throughput: (bytes * 8) / (time_ms * 1000) = Mbps
        throughputRecvMbps = (bytesReceived * 8.0) / (total_time * 1000.0);
        std::cout << "[DEBUG] Throughput calculated: Sent=" << throughputSentMbps 
                  << " Mbps, Recv=" << throughputRecvMbps << " Mbps" << std::endl;
    } else {
        std::cout << "[DEBUG] Total time is 0, throughput cannot be calculated" << std::endl;
    }
    
    // Update labels with current values
    avgThroughputSentLabel->setText(QString("%1 Mbps").arg(throughputSentMbps, 0, 'f', 2));
    avgThroughputRecvLabel->setText(QString("%1 Mbps").arg(throughputRecvMbps, 0, 'f', 2));
    
    // Packet loss rate with failed/total
    uint64_t total = metrics.total_requests.load();
    uint64_t failed = metrics.failed_requests.load();
    double lossRate = (total > 0) ? (failed * 100.0 / total) : 0.0;
    
    packetLossLabel->setText(QString("%1% (%2/%3)")
        .arg(lossRate, 0, 'f', 2)
        .arg(failed)
        .arg(total));
}

void ClientWindow::parseFileListResponse(const std::string& response) {
    // Parse the LIST response and update fileListWidget
    fileListWidget->clear();
    
    // Split response by newlines and parse each file entry
    std::istringstream iss(response);
    std::string line;
    
    while (std::getline(iss, line)) {
        if (!line.empty() && line != "LIST_OK" && line != "LIST_END") {
            // Determine icon based on file extension
            QString icon = "📄";
            if (line.find(".png") != std::string::npos || 
                line.find(".jpg") != std::string::npos ||
                line.find(".jpeg") != std::string::npos) {
                icon = "🖼";
            } else if (line.find(".csv") != std::string::npos ||
                       line.find(".xls") != std::string::npos) {
                icon = "📊";
            } else if (line.find(".zip") != std::string::npos ||
                       line.find(".tar") != std::string::npos) {
                icon = "📦";
            }
            
            fileListWidget->addItem(QString("%1 %2").arg(icon).arg(QString::fromStdString(line)));
        }
    }
    
    if (fileListWidget->count() == 0) {
        QListWidgetItem* noFilesItem = new QListWidgetItem("No files available");
        noFilesItem->setForeground(QColor("#999"));
        noFilesItem->setFlags(Qt::NoItemFlags);
        fileListWidget->addItem(noFilesItem);
    }
}
