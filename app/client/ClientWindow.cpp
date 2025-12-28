#include "ClientWindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QFileInfo>
#include <QTimer>

ClientWindow::ClientWindow(QWidget *parent)
    : QMainWindow(parent), client(std::make_unique<Client>()), isConnected(false), currentPort(0) {
    
    setWindowTitle("File Transfer Client");
    resize(600, 700);
    
    setupUI();
    
    // Update timer
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &ClientWindow::updateUI);
    updateTimer->start(500); // Update every 500ms
}

ClientWindow::~ClientWindow() {
    if (client && client->isConnected()) {
        client->disconnect();
    }
}

void ClientWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // ===== Status Bar =====
    QWidget *statusWidget = new QWidget();
    QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(10, 5, 10, 5);
    
    statusLabel = new QLabel("Status: Disconnected");
    statusLabel->setStyleSheet("QLabel { background-color: #d9534f; color: white; padding: 5px; border-radius: 3px; font-weight: bold; }");
    
    QLabel *serverLabel = new QLabel("Not connected to server");
    serverLabel->setStyleSheet("QLabel { background-color: #5cb85c; color: white; padding: 5px; border-radius: 3px; }");
    serverLabel->setObjectName("serverLabel");
    
    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(serverLabel);
    statusLayout->addStretch();
    
    mainLayout->addWidget(statusWidget);
    
    // ===== Command Section =====
    QWidget *commandWidget = new QWidget();
    QHBoxLayout *commandLayout = new QHBoxLayout(commandWidget);
    
    QLabel *cmdLabel = new QLabel("Command:");
    commandCombo = new QComboBox();
    commandCombo->setEditable(true);
    commandCombo->addItems({
        "connect 192.168.1.100 99000",
        "connect 127.0.0.1 8080",
        "connect 127.0.0.1 9000"
    });
    commandCombo->setMinimumWidth(300);
    
    connectBtn = new QPushButton("Connect");
    connectBtn->setStyleSheet("QPushButton { background-color: #0275d8; color: white; padding: 5px 15px; font-weight: bold; } QPushButton:hover { background-color: #025aa5; }");
    connect(connectBtn, &QPushButton::clicked, this, &ClientWindow::onConnectClicked);
    
    commandLayout->addWidget(cmdLabel);
    commandLayout->addWidget(commandCombo);
    commandLayout->addWidget(connectBtn);
    
    mainLayout->addWidget(commandWidget);
    
    // ===== Tab Widget =====
    tabWidget = new QTabWidget();
    
    // Server Files Tab
    fileListWidget = new QListWidget();
    fileListWidget->setAlternatingRowColors(true);
    tabWidget->addTab(fileListWidget, "Server Files");
    
    // Transfer Log Tab
    logTextEdit = new QTextEdit();
    logTextEdit->setReadOnly(true);
    logTextEdit->setStyleSheet("QTextEdit { font-family: 'Courier New', monospace; }");
    tabWidget->addTab(logTextEdit, "Transfer Log");
    
    mainLayout->addWidget(tabWidget);
    
    // ===== Action Buttons =====
    QWidget *btnWidget = new QWidget();
    QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
    
    disconnectBtn = new QPushButton("Disconnect");
    disconnectBtn->setStyleSheet("QPushButton { background-color: #d9534f; color: white; } QPushButton:disabled { background-color: #cccccc; }");
    disconnectBtn->setEnabled(false);
    connect(disconnectBtn, &QPushButton::clicked, this, &ClientWindow::onDisconnectClicked);
    
    listFilesBtn = new QPushButton("List Files");
    listFilesBtn->setStyleSheet("QPushButton { background-color: #0275d8; color: white; } QPushButton:disabled { background-color: #cccccc; }");
    listFilesBtn->setEnabled(false);
    connect(listFilesBtn, &QPushButton::clicked, this, &ClientWindow::onListFilesClicked);
    
    downloadBtn = new QPushButton("Download File");
    downloadBtn->setStyleSheet("QPushButton { background-color: #5bc0de; color: white; } QPushButton:disabled { background-color: #cccccc; }");
    downloadBtn->setEnabled(false);
    connect(downloadBtn, &QPushButton::clicked, this, &ClientWindow::onDownloadFileClicked);
    
    uploadBtn = new QPushButton("Upload File");
    uploadBtn->setStyleSheet("QPushButton { background-color: #5cb85c; color: white; } QPushButton:disabled { background-color: #cccccc; }");
    uploadBtn->setEnabled(false);
    connect(uploadBtn, &QPushButton::clicked, this, &ClientWindow::onUploadFileClicked);
    
    metricsBtn = new QPushButton("Metrics");
    metricsBtn->setStyleSheet("QPushButton { background-color: #f0ad4e; color: white; }");
    connect(metricsBtn, &QPushButton::clicked, this, &ClientWindow::onMetricsClicked);
    
    exportBtn = new QPushButton("Export CSV");
    exportBtn->setStyleSheet("QPushButton { background-color: #5bc0de; color: white; }");
    connect(exportBtn, &QPushButton::clicked, this, &ClientWindow::onExportCSVClicked);
    
    btnLayout->addWidget(disconnectBtn);
    btnLayout->addWidget(listFilesBtn);
    btnLayout->addWidget(downloadBtn);
    btnLayout->addWidget(uploadBtn);
    btnLayout->addWidget(metricsBtn);
    btnLayout->addWidget(exportBtn);
    
    mainLayout->addWidget(btnWidget);
    
    // ===== Client Metrics =====
    QGroupBox *metricsGroup = new QGroupBox("Client Metrics");
    QGridLayout *metricsLayout = new QGridLayout();
    
    metricsLayout->addWidget(new QLabel("Bytes Sent:"), 0, 0);
    bytesSentLabel = new QLabel("0 B");
    bytesSentLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsLayout->addWidget(bytesSentLabel, 0, 1);
    
    metricsLayout->addWidget(new QLabel("Bytes Received:"), 0, 2);
    bytesReceivedLabel = new QLabel("0 B");
    bytesReceivedLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsLayout->addWidget(bytesReceivedLabel, 0, 3);
    
    metricsLayout->addWidget(new QLabel("Files Uploaded:"), 1, 0);
    filesUploadedLabel = new QLabel("0");
    filesUploadedLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsLayout->addWidget(filesUploadedLabel, 1, 1);
    
    metricsLayout->addWidget(new QLabel("Files Downloaded:"), 1, 2);
    filesDownloadedLabel = new QLabel("0");
    filesDownloadedLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsLayout->addWidget(filesDownloadedLabel, 1, 3);
    
    metricsLayout->addWidget(new QLabel("Avg Throughput:"), 2, 0);
    avgThroughputSendLabel = new QLabel("-");
    avgThroughputSendLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsLayout->addWidget(avgThroughputSendLabel, 2, 1);
    
    metricsLayout->addWidget(new QLabel("Avg Throughput:"), 2, 2);
    avgThroughputRecvLabel = new QLabel("8.1 MB/s");
    avgThroughputRecvLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsLayout->addWidget(avgThroughputRecvLabel, 2, 3);
    
    metricsGroup->setLayout(metricsLayout);
    mainLayout->addWidget(metricsGroup);
    
    // ===== Quit Button =====
    QPushButton *quitBtn = new QPushButton("Quit");
    quitBtn->setStyleSheet("QPushButton { background-color: #d9534f; color: white; padding: 8px; font-weight: bold; }");
    quitBtn->setMaximumWidth(150);
    connect(quitBtn, &QPushButton::clicked, this, &ClientWindow::onQuitClicked);
    
    QHBoxLayout *quitLayout = new QHBoxLayout();
    quitLayout->addStretch();
    quitLayout->addWidget(quitBtn);
    
    mainLayout->addLayout(quitLayout);
}

void ClientWindow::onConnectClicked() {
    QString command = commandCombo->currentText();
    QStringList parts = command.split(' ', Qt::SkipEmptyParts);
    
    if (parts.size() != 3 || parts[0] != "connect") {
        QMessageBox::warning(this, "Invalid Command", "Format: connect <IP> <PORT>");
        return;
    }
    
    QString ip = parts[1];
    bool ok;
    quint16 port = parts[2].toUShort(&ok);
    
    if (!ok) {
        QMessageBox::warning(this, "Invalid Port", "Port must be a valid number");
        return;
    }
    
    logMessage("Connecting to " + ip + ":" + QString::number(port) + "...", true);
    
    if (client->connect(ip.toStdString(), port)) {
        isConnected = true;
        currentServer = ip;
        currentPort = port;
        
        logMessage("✓ Connected to " + ip + ":" + QString::number(port), true);
        
        updateStatusBar();
        
        // Enable buttons
        connectBtn->setEnabled(false);
        disconnectBtn->setEnabled(true);
        listFilesBtn->setEnabled(true);
        downloadBtn->setEnabled(true);
        uploadBtn->setEnabled(true);
    } else {
        logMessage("✗ Failed to connect to server", false);
        QMessageBox::critical(this, "Connection Failed", "Could not connect to " + ip + ":" + QString::number(port));
    }
}

void ClientWindow::onDisconnectClicked() {
    if (client && client->isConnected()) {
        client->disconnect();
        isConnected = false;
        
        logMessage("Disconnected from server", true);
        
        // Update UI
        connectBtn->setEnabled(true);
        disconnectBtn->setEnabled(false);
        listFilesBtn->setEnabled(false);
        downloadBtn->setEnabled(false);
        uploadBtn->setEnabled(false);
        
        fileListWidget->clear();
        updateStatusBar();
    }
}

void ClientWindow::onListFilesClicked() {
    if (!client->isConnected()) {
        QMessageBox::warning(this, "Not Connected", "Please connect to server first");
        return;
    }
    
    logMessage("Listing files on server...", true);
    
    // Clear current list
    fileListWidget->clear();
    
    // Get actual file list from server
    auto files = client->getFileList();
    if (!files.empty()) {
        logMessage("✓ Found " + QString::number(files.size()) + " file(s)", true);
        
        // Add actual files with icons
        for (const auto& filename : files) {
            QString qFilename = QString::fromStdString(filename);
            QString icon = "📄"; // Default
            
            if (qFilename.endsWith(".pdf")) icon = "📄";
            else if (qFilename.endsWith(".png") || qFilename.endsWith(".jpg") || qFilename.endsWith(".jpeg")) icon = "🖼️";
            else if (qFilename.endsWith(".txt")) icon = "📃";
            else if (qFilename.endsWith(".zip") || qFilename.endsWith(".tar") || qFilename.endsWith(".gz")) icon = "🗃️";
            else if (qFilename.endsWith(".mp3") || qFilename.endsWith(".wav")) icon = "🎵";
            else if (qFilename.endsWith(".mp4") || qFilename.endsWith(".avi")) icon = "🎥";
            
            fileListWidget->addItem(icon + " " + qFilename);
        }
    } else {
        logMessage("✓ No files on server", true);
        fileListWidget->addItem("No files available");
    }
}

void ClientWindow::onDownloadFileClicked() {
    QListWidgetItem *item = fileListWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "No File Selected", "Please select a file to download");
        return;
    }
    
    QString filename = item->text();
    // Remove emoji
    filename = filename.mid(filename.indexOf(' ') + 1);
    
    QString saveDir = QFileDialog::getExistingDirectory(this, "Select Download Directory", QDir::homePath());
    if (saveDir.isEmpty()) {
        return;
    }
    
    logMessage("Downloading " + filename + "...", true);
    
    // getFile(filename, saveDir) will save as saveDir/filename
    if (client->getFile(filename.toStdString(), saveDir.toStdString())) {
        QString fullPath = saveDir + "/" + filename;
        logMessage("✓ Download completed: " + fullPath, true);
        QMessageBox::information(this, "Success", "File downloaded to:\n" + fullPath);
        updateMetrics();
    } else {
        logMessage("✗ Download failed", false);
        QMessageBox::critical(this, "Error", "Failed to download file");
    }
}

void ClientWindow::onUploadFileClicked() {
    QString filename = QFileDialog::getOpenFileName(this, "Select File to Upload", QDir::homePath());
    if (filename.isEmpty()) {
        return;
    }
    
    QFileInfo fileInfo(filename);
    logMessage("Uploading " + fileInfo.fileName() + "...", true);
    
    // putFile uploads to server's shared directory
    if (client->putFile(filename.toStdString())) {
        logMessage("✓ Upload completed: " + fileInfo.fileName() + " → server shared directory", true);
        QMessageBox::information(this, "Success", "File uploaded to server!\nUse 'List Files' to see it.");
        updateMetrics();
        // Auto refresh file list
        QTimer::singleShot(500, this, &ClientWindow::onListFilesClicked);
    } else {
        logMessage("✗ Upload failed", false);
        QMessageBox::critical(this, "Error", "Failed to upload file");
    }
}

void ClientWindow::onMetricsClicked() {
    updateMetrics();
    tabWidget->setCurrentIndex(1); // Switch to log tab
    logMessage("=== Client Metrics Updated ===", true);
}

void ClientWindow::onExportCSVClicked() {
    QString filename = QFileDialog::getSaveFileName(this, "Export Metrics", "client_metrics.csv", "CSV Files (*.csv)");
    if (filename.isEmpty()) {
        return;
    }
    
    if (client->exportMetrics(filename.toStdString())) {
        QMessageBox::information(this, "Success", "Metrics exported to " + filename);
        logMessage("Metrics exported to: " + filename, true);
    } else {
        QMessageBox::critical(this, "Error", "Failed to export metrics");
    }
}

void ClientWindow::onQuitClicked() {
    if (client && client->isConnected()) {
        client->disconnect();
    }
    QApplication::quit();
}

void ClientWindow::updateUI() {
    updateMetrics();
}

void ClientWindow::updateStatusBar() {
    QLabel *serverLabel = findChild<QLabel*>("serverLabel");
    
    if (isConnected) {
        statusLabel->setText("Status: Connected");
        statusLabel->setStyleSheet("QLabel { background-color: #5cb85c; color: white; padding: 5px; border-radius: 3px; font-weight: bold; }");
        
        if (serverLabel) {
            serverLabel->setText("Connected to " + currentServer + ":" + QString::number(currentPort));
        }
    } else {
        statusLabel->setText("Status: Disconnected");
        statusLabel->setStyleSheet("QLabel { background-color: #d9534f; color: white; padding: 5px; border-radius: 3px; font-weight: bold; }");
        
        if (serverLabel) {
            serverLabel->setText("Not connected to server");
        }
    }
}

void ClientWindow::updateMetrics() {
    if (!client) return;
    
    const auto& metrics = client->getMetrics();
    
    // Update transfer metrics
    bytesSentLabel->setText(QString::number(metrics.totalBytesSent / 1024.0 / 1024.0, 'f', 2) + " MB");
    bytesReceivedLabel->setText(QString::number(metrics.totalBytesReceived / 1024.0 / 1024.0, 'f', 2) + " MB");
    filesUploadedLabel->setText(QString::number(metrics.filesUploaded));
    filesDownloadedLabel->setText(QString::number(metrics.filesDownloaded));
    
    // Display RTT and packet loss
    if (metrics.rtt_ms > 0) {
        double throughput_mbps = metrics.throughput_kbps / 1024.0;
        avgThroughputSendLabel->setText(QString::number(throughput_mbps, 'f', 2) + " MB/s");
        avgThroughputRecvLabel->setText(QString::number(throughput_mbps, 'f', 2) + " MB/s | RTT: " + 
                                       QString::number(metrics.rtt_ms, 'f', 1) + "ms | Loss: " + 
                                       QString::number(metrics.packet_loss_rate, 'f', 2) + "%");
    } else {
        avgThroughputSendLabel->setText("0 MB/s");
        avgThroughputRecvLabel->setText("0 MB/s | RTT: -ms | Loss: 0%");
    }
}

void ClientWindow::logMessage(const QString& message, bool success) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString color = success ? "green" : "red";
    QString icon = success ? "✓" : "✗";
    
    logTextEdit->append(QString("<span style='color: gray;'>[%1]</span> <span style='color: %2;'>%3 %4</span>")
                        .arg(timestamp)
                        .arg(color)
                        .arg(icon)
                        .arg(message));
}

void ClientWindow::onCommandExecute() {
    // For future use if needed
}
