#include "ServerWindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDateTime>
#include <QGridLayout>
#include <QNetworkInterface>
#include <sstream>
#include <iostream>

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent), 
      server(std::make_unique<Server>()),
      serverThread(nullptr),
      currentPort(9000),
      currentSharedDir("./shared"),
      verboseMode(false),
      previousFilesSent_(0),
      previousFilesReceived_(0),
      previousTotalConnections_(0) {
    setupUI();
    
    // Setup status update timer
    statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &ServerWindow::updateStatus);
    statusTimer->start(1000); // Update every second
    
    // Auto-start server on port 9000
    startServer(9000, "./shared");
}

ServerWindow::~ServerWindow() {
    stopServer();
}

void ServerWindow::setupUI() {
    setWindowTitle("File Transfer Server");
    setMinimumSize(800, 650);
    
    // Apply light theme styling
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
        QLabel#statusBarStopped {
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
        QPushButton#stopServerButton {
            background-color: #f44336;
            font-size: 12pt;
        }
        QPushButton#stopServerButton:hover {
            background-color: #d32f2f;
        }
        QPushButton#verboseButton {
            background-color: #607D8B;
        }
        QPushButton#verboseButton:hover {
            background-color: #546E7A;
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
            padding: 8px;
        }
        QListWidget::item:selected {
            background-color: #2196F3;
            color: white;
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
    statusLabel = new QLabel("Status: Running", this);
    statusLabel->setObjectName("statusBar");
    mainLayout->addWidget(statusLabel);
    
    serverIPLabel = new QLabel(QString("Server IP: %1").arg(getServerIP()), this);
    serverIPLabel->setStyleSheet("QLabel { color: #666; padding: 5px; background-color: #e3f2fd; }");
    mainLayout->addWidget(serverIPLabel);
    
    uptimeLabel = new QLabel("Uptime: 00:00:00", this);
    uptimeLabel->setStyleSheet("QLabel { color: #666; padding: 5px; background-color: #fff3e0; font-weight: bold; }");
    mainLayout->addWidget(uptimeLabel);
    
    connectionLabel = new QLabel("Listening on Port 9000 | Shared Directory: /shared", this);
    connectionLabel->setStyleSheet("QLabel { color: #666; padding: 5px; }");
    mainLayout->addWidget(connectionLabel);
    
    // Command section
    QHBoxLayout* commandLayout = new QHBoxLayout();
    QLabel* cmdLabel = new QLabel("Command:", this);
    commandLayout->addWidget(cmdLabel);
    
    commandCombo = new QComboBox(this);
    commandCombo->setEditable(true);
    commandCombo->addItem("clients");
    commandCombo->addItem("metrics");
    commandCombo->addItem("stop");
    commandCombo->addItem("start <port>");
    commandCombo->addItem("dir <path>");
    commandCombo->addItem("verbose");
    commandCombo->addItem("reset");
    commandCombo->setCurrentIndex(0);
    commandLayout->addWidget(commandCombo, 1);
    
    executeButton = new QPushButton("Execute", this);
    connect(executeButton, &QPushButton::clicked, this, &ServerWindow::onExecuteCommand);
    commandLayout->addWidget(executeButton);
    
    mainLayout->addLayout(commandLayout);
    
    // Tab widget for Active Clients and Server Metrics
    tabWidget = new QTabWidget(this);
    
    // Active Clients tab
    activeClientsWidget = new QListWidget(this);
    tabWidget->addTab(activeClientsWidget, "Active Clients");
    
    // Transfer Log tab
    transferLogText = new QTextEdit(this);
    transferLogText->setReadOnly(true);
    tabWidget->addTab(transferLogText, "Transfer Log");
    
    // Server Metrics tab - using a widget with labels
    QWidget* metricsWidget = new QWidget(this);
    QVBoxLayout* metricsTabLayout = new QVBoxLayout(metricsWidget);
    metricsTabLayout->setContentsMargins(20, 20, 20, 20);
    metricsTabLayout->setSpacing(15);
    
    QGridLayout* metricsGrid = new QGridLayout();
    metricsGrid->setSpacing(10);
    
    QLabel* acLabel = new QLabel("Active Clients:", this);
    acLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsGrid->addWidget(acLabel, 0, 0);
    activeClientsCountLabel = new QLabel("0", this);
    activeClientsCountLabel->setStyleSheet("QLabel { font-size: 18pt; font-weight: normal; }");
    metricsGrid->addWidget(activeClientsCountLabel, 0, 1);
    
    QLabel* tcLabel = new QLabel("Total Connections:", this);
    tcLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsGrid->addWidget(tcLabel, 1, 0);
    totalConnectionsLabel = new QLabel("0", this);
    totalConnectionsLabel->setStyleSheet("QLabel { font-size: 18pt; font-weight: normal; }");
    metricsGrid->addWidget(totalConnectionsLabel, 1, 1);
    
    QLabel* fsLabel = new QLabel("Files Sent:", this);
    fsLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsGrid->addWidget(fsLabel, 2, 0);
    filesSentLabel = new QLabel("0", this);
    filesSentLabel->setStyleSheet("QLabel { font-size: 18pt; font-weight: normal; }");
    metricsGrid->addWidget(filesSentLabel, 2, 1);
    
    QLabel* frLabel = new QLabel("Files Received:", this);
    frLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsGrid->addWidget(frLabel, 3, 0);
    filesReceivedLabel = new QLabel("0", this);
    filesReceivedLabel->setStyleSheet("QLabel { font-size: 18pt; font-weight: normal; }");
    metricsGrid->addWidget(filesReceivedLabel, 3, 1);
    
    QLabel* dsLabel = new QLabel("Total Data Sent:", this);
    dsLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsGrid->addWidget(dsLabel, 4, 0);
    dataSentLabel = new QLabel("0.00 MB", this);
    dataSentLabel->setStyleSheet("QLabel { font-size: 18pt; font-weight: normal; }");
    metricsGrid->addWidget(dataSentLabel, 4, 1);
    
    QLabel* drLabel = new QLabel("Total Data Received:", this);
    drLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsGrid->addWidget(drLabel, 5, 0);
    dataReceivedLabel = new QLabel("0.00 MB", this);
    dataReceivedLabel->setStyleSheet("QLabel { font-size: 18pt; font-weight: normal; }");
    metricsGrid->addWidget(dataReceivedLabel, 5, 1);
    
    QLabel* plLabel = new QLabel("Packet Loss Rate:", this);
    plLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsGrid->addWidget(plLabel, 6, 0);
    packetLossLabel = new QLabel("0.00% (0/0)", this);
    packetLossLabel->setStyleSheet("QLabel { font-size: 14pt; font-weight: normal; }");
    metricsGrid->addWidget(packetLossLabel, 6, 1);
    
    QLabel* atLabel = new QLabel("Avg Throughput:", this);
    atLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsGrid->addWidget(atLabel, 7, 0);
    avgThroughputLabel = new QLabel("0.00 Mbps", this);
    avgThroughputLabel->setStyleSheet("QLabel { font-size: 14pt; font-weight: normal; }");
    metricsGrid->addWidget(avgThroughputLabel, 7, 1);
    
    QLabel* ptLabel = new QLabel("Peak Throughput:", this);
    ptLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsGrid->addWidget(ptLabel, 8, 0);
    peakThroughputLabel = new QLabel("0.00 Mbps", this);
    peakThroughputLabel->setStyleSheet("QLabel { font-size: 14pt; font-weight: normal; }");
    metricsGrid->addWidget(peakThroughputLabel, 8, 1);
    
    QLabel* alLabel = new QLabel("Avg Latency:", this);
    alLabel->setStyleSheet("QLabel { font-weight: bold; }");
    metricsGrid->addWidget(alLabel, 9, 0);
    avgLatencyLabel = new QLabel("0.00 ms", this);
    avgLatencyLabel->setStyleSheet("QLabel { font-size: 14pt; font-weight: normal; }");
    metricsGrid->addWidget(avgLatencyLabel, 9, 1);
    
    metricsTabLayout->addLayout(metricsGrid);
    metricsTabLayout->addStretch();
    
    tabWidget->addTab(metricsWidget, "Server Metrics");
    
    mainLayout->addWidget(tabWidget, 1);
    
    // Action buttons row 1
    QHBoxLayout* buttonsRow1 = new QHBoxLayout();
    
    metricsButton = new QPushButton("≡ Metrics", this);
    connect(metricsButton, &QPushButton::clicked, this, &ServerWindow::onMetricsClicked);
    buttonsRow1->addWidget(metricsButton);
    
    clientsButton = new QPushButton("👤 Clients", this);
    connect(clientsButton, &QPushButton::clicked, this, &ServerWindow::onClientsClicked);
    buttonsRow1->addWidget(clientsButton);
    
    changeDirButton = new QPushButton("📁 Change Dir", this);
    connect(changeDirButton, &QPushButton::clicked, this, &ServerWindow::onChangeDirClicked);
    buttonsRow1->addWidget(changeDirButton);
    
    mainLayout->addLayout(buttonsRow1);
    
    // Action buttons row 2
    QHBoxLayout* buttonsRow2 = new QHBoxLayout();
    
    verboseButton = new QPushButton("◯ Verbose", this);
    verboseButton->setObjectName("verboseButton");
    connect(verboseButton, &QPushButton::clicked, this, &ServerWindow::onVerboseClicked);
    buttonsRow2->addWidget(verboseButton);
    
    exportButton = new QPushButton("Export CSV", this);
    connect(exportButton, &QPushButton::clicked, this, &ServerWindow::onExportClicked);
    buttonsRow2->addWidget(exportButton);
    
    mainLayout->addLayout(buttonsRow2);
    
    // Stop Server button
    stopServerButton = new QPushButton("Stop Server", this);
    stopServerButton->setObjectName("stopServerButton");
    stopServerButton->setMinimumHeight(50);
    connect(stopServerButton, &QPushButton::clicked, this, &ServerWindow::onStopServerClicked);
    mainLayout->addWidget(stopServerButton);
    
    // Quit button
    quitButton = new QPushButton("Quit", this);
    quitButton->setObjectName("quitButton");
    connect(quitButton, &QPushButton::clicked, this, &ServerWindow::onQuitClicked);
    mainLayout->addWidget(quitButton);
    
    setCentralWidget(centralWidget);
    
    appendLog("Server initialized", "green");
}

void ServerWindow::appendLog(const QString& text, const QString& color) {
    QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss] ");
    QString coloredText = QString("<span style='color: gray;'>%1</span><span style='color: %2;'>%3</span><br>")
                              .arg(timestamp)
                              .arg(color)
                              .arg(text.toHtmlEscaped());
    transferLogText->moveCursor(QTextCursor::End);
    transferLogText->insertHtml(coloredText);
    transferLogText->moveCursor(QTextCursor::End);
}

QString ServerWindow::getServerIP() {
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback()) {
            return address.toString();
        }
    }
    return "127.0.0.1";
}

void ServerWindow::startServer(int port, const QString& sharedDir) {
    if (server->isRunning()) {
        return;
    }
    
    currentPort = port;
    currentSharedDir = sharedDir;
    
    if (server->start(static_cast<uint16_t>(port), sharedDir.toStdString())) {
        serverStartTime = QDateTime::currentDateTime();
        appendLog(QString("Server started on port %1").arg(port), "green");
        appendLog(QString("Shared directory: %1").arg(sharedDir), "green");
        
        // Start server in separate thread
        serverThread = new ServerThread(server.get(), this);
        
        // Connect signal to receive server logs
        connect(serverThread, &ServerThread::logMessage, this, &ServerWindow::onServerLog, Qt::QueuedConnection);
        
        serverThread->start();
        
        appendLog("Waiting for client connections...", "blue");
    } else {
        appendLog("Failed to start server", "red");
    }
}

void ServerWindow::stopServer() {
    if (!server->isRunning()) {
        return;
    }
    
    appendLog("Stopping server...", "orange");
    server->stop();
    
    if (serverThread) {
        serverThread->wait(3000);
        if (serverThread->isRunning()) {
            serverThread->terminate();
            serverThread->wait();
        }
        delete serverThread;
        serverThread = nullptr;
    }
    
    appendLog("Server stopped", "red");
}

void ServerWindow::onExecuteCommand() {
    QString command = commandCombo->currentText().trimmed();
    if (command.isEmpty()) return;
    
    processCommand(command);
}

void ServerWindow::processCommand(const QString& command) {
    QStringList parts = command.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) return;
    
    QString cmd = parts[0].toLower();
    
    if (cmd == "clients") {
        onClientsClicked();
    }
    else if (cmd == "metrics") {
        onMetricsClicked();
    }
    else if (cmd == "stop") {
        stopServer();
    }
    else if (cmd == "start") {
        int port = 9000;
        if (parts.size() >= 2) {
            bool ok;
            port = parts[1].toInt(&ok);
            if (!ok) port = 9000;
        }
        startServer(port, currentSharedDir);
    }
    else if (cmd == "dir") {
        onChangeDirClicked();
    }
    else if (cmd == "verbose") {
        onVerboseClicked();
    }
    else if (cmd == "reset") {
        server->resetMetrics();
        updateMetrics();
    }
}

void ServerWindow::onMetricsClicked() {
    tabWidget->setCurrentIndex(2); // Switch to metrics tab
    updateMetrics();
}

void ServerWindow::onClientsClicked() {
    tabWidget->setCurrentIndex(0); // Switch to clients tab
    updateClientsList();
}

void ServerWindow::onChangeDirClicked() {
    QString dir = QFileDialog::getExistingDirectory(this, 
        "Select Shared Directory", currentSharedDir);
    
    if (!dir.isEmpty()) {
        if (server->setSharedDirectory(dir.toStdString())) {
            currentSharedDir = dir;
            QMessageBox::information(this, "Success", 
                QString("Shared directory changed to:\n%1").arg(dir));
        } else {
            QMessageBox::warning(this, "Error", 
                "Failed to change shared directory.");
        }
    }
}

void ServerWindow::onVerboseClicked() {
    verboseMode = !verboseMode;
    server->setVerbose(verboseMode);
    
    if (verboseMode) {
        verboseButton->setText("◉ Verbose");
    } else {
        verboseButton->setText("◯ Verbose");
    }
}

void ServerWindow::onExportClicked() {
    QString filename = QFileDialog::getSaveFileName(this, "Export Server Metrics",
        QString("server_metrics_%1.csv").arg(QDateTime::currentSecsSinceEpoch()),
        "CSV Files (*.csv)");
    
    if (!filename.isEmpty()) {
        if (server->exportMetrics(filename.toStdString())) {
            QMessageBox::information(this, "Success", 
                QString("Metrics successfully exported to:\n%1").arg(filename));
        } else {
            QMessageBox::warning(this, "Error", "Failed to export metrics.");
        }
    }
}

void ServerWindow::onStopServerClicked() {
    if (server->isRunning()) {
        int ret = QMessageBox::question(this, "Stop Server",
                                       "Are you sure you want to stop the server?",
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            stopServer();
        }
    } else {
        bool ok;
        int port = QInputDialog::getInt(this, "Start Server",
                                       "Enter port number:", currentPort, 1, 65535, 1, &ok);
        if (ok) {
            startServer(port, currentSharedDir);
        }
    }
}

void ServerWindow::onQuitClicked() {
    if (server->isRunning()) {
        int ret = QMessageBox::question(this, "Quit",
                                       "Server is running. Stop and quit?",
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            stopServer();
            close();
        }
    } else {
        close();
    }
}

void ServerWindow::updateStatus() {
    updateStatusBar();
    updateMetrics();
    updateClientsList();
}

void ServerWindow::updateStatusBar() {
    if (server->isRunning()) {
        statusLabel->setText("Status: Running");
        statusLabel->setObjectName("statusBar");
        statusLabel->setStyleSheet("QLabel { background-color: #4CAF50; color: white; padding: 8px; font-weight: bold; }");
        
        // Calculate uptime
        qint64 uptime = serverStartTime.secsTo(QDateTime::currentDateTime());
        int days = uptime / 86400;
        int hours = (uptime % 86400) / 3600;
        int minutes = (uptime % 3600) / 60;
        int seconds = uptime % 60;
        
        QString uptimeStr;
        if (days > 0) {
            uptimeStr = QString("Uptime: %1d %2h %3m %4s")
                .arg(days).arg(hours).arg(minutes).arg(seconds);
        } else {
            uptimeStr = QString("Uptime: %1:%2:%3")
                .arg(hours, 2, 10, QChar('0'))
                .arg(minutes, 2, 10, QChar('0'))
                .arg(seconds, 2, 10, QChar('0'));
        }
        uptimeLabel->setText(uptimeStr);
        uptimeLabel->setStyleSheet("QLabel { color: #4CAF50; padding: 5px; background-color: #e8f5e9; font-weight: bold; }");
        
        serverIPLabel->setText(QString("Server IP: %1").arg(getServerIP()));
        connectionLabel->setText(QString("Listening on Port %1 | Shared Directory: %2")
                                    .arg(currentPort)
                                    .arg(currentSharedDir));
        
        stopServerButton->setText("Stop Server");
        stopServerButton->setStyleSheet("QPushButton { background-color: #f44336; font-size: 12pt; }");
    } else {
        statusLabel->setText("Status: Stopped");
        statusLabel->setObjectName("statusBarStopped");
        statusLabel->setStyleSheet("QLabel { background-color: #f44336; color: white; padding: 8px; font-weight: bold; }");
        
        uptimeLabel->setText("Uptime: --:--:--");
        uptimeLabel->setStyleSheet("QLabel { color: #666; padding: 5px; background-color: #fff3e0; font-weight: bold; }");
        
        serverIPLabel->setText(QString("Server IP: %1").arg(getServerIP()));
        connectionLabel->setText("Server is not running");
        
        stopServerButton->setText("Start Server");
        stopServerButton->setStyleSheet("QPushButton { background-color: #4CAF50; font-size: 12pt; }");
    }
}

void ServerWindow::updateMetrics() {
    const ServerMetrics& metrics = server->getMetrics();
    
    size_t activeCount = server->getActiveSessionCount();
    uint64_t currentFilesSent = metrics.filesDownloaded.load();
    uint64_t currentFilesReceived = metrics.filesUploaded.load();
    uint64_t currentTotalConnections = metrics.totalConnections.load();
    
    // Detect file downloads (GET)
    if (currentFilesSent > previousFilesSent_) {
        uint64_t diff = currentFilesSent - previousFilesSent_;
        appendLog(QString("✓ File(s) sent to client (GET): %1").arg(diff), "blue");
        previousFilesSent_ = currentFilesSent;
    }
    
    // Detect file uploads (PUT)
    if (currentFilesReceived > previousFilesReceived_) {
        uint64_t diff = currentFilesReceived - previousFilesReceived_;
        appendLog(QString("✓ File(s) received from client (PUT): %1").arg(diff), "blue");
        previousFilesReceived_ = currentFilesReceived;
    }
    
    // Detect new requests (including LIST)
    if (currentTotalConnections > previousTotalConnections_) {
        // If total connections increased but no file transfer, it's likely a LIST request
        bool noFileTransfer = (currentFilesSent == previousFilesSent_) && 
                              (currentFilesReceived == previousFilesReceived_);
        if (noFileTransfer && !previousClients_.empty()) {
            appendLog(QString("✓ Client request received (LIST)"), "blue");
        }
        previousTotalConnections_ = currentTotalConnections;
    }
    
    activeClientsCountLabel->setText(QString::number(activeCount));
    totalConnectionsLabel->setText(QString::number(currentTotalConnections));
    filesSentLabel->setText(QString::number(currentFilesSent));
    filesReceivedLabel->setText(QString::number(currentFilesReceived));
    
    double sentMB = metrics.totalBytesSent.load() / 1024.0 / 1024.0;
    double recvMB = metrics.totalBytesReceived.load() / 1024.0 / 1024.0;
    
    dataSentLabel->setText(QString("%1 MB").arg(sentMB, 0, 'f', 2));
    dataReceivedLabel->setText(QString("%1 MB").arg(recvMB, 0, 'f', 2));
    
    // Calculate packet loss rate
    uint64_t totalConn = metrics.totalConnections.load();
    uint64_t failedConn = metrics.failedConnections.load();
    double lossRate = (totalConn > 0) ? (failedConn * 100.0 / totalConn) : 0.0;
    packetLossLabel->setText(QString("%1% (%2/%3)")
        .arg(lossRate, 0, 'f', 2)
        .arg(failedConn)
        .arg(totalConn));
    
    // Throughput in Mbps (convert from kbps)
    double avgThroughputMbps = metrics.averageThroughput_kbps / 1000.0;
    double peakThroughputMbps = metrics.peakThroughput_kbps / 1000.0;
    
    avgThroughputLabel->setText(QString("%1 Mbps").arg(avgThroughputMbps, 0, 'f', 2));
    peakThroughputLabel->setText(QString("%1 Mbps").arg(peakThroughputMbps, 0, 'f', 2));
    avgLatencyLabel->setText(QString("%1 ms").arg(metrics.averageLatency_ms, 0, 'f', 2));
}

void ServerWindow::updateClientsList() {
    activeClientsWidget->clear();
    std::vector<std::string> clients = server->getActiveClients();
    
    // Convert to set for easier comparison
    std::set<std::string> currentClients(clients.begin(), clients.end());
    
    // Detect new connections
    for (const auto& client : currentClients) {
        if (previousClients_.find(client) == previousClients_.end()) {
            appendLog(QString("Client connected: %1").arg(QString::fromStdString(client)), "green");
        }
    }
    
    // Detect disconnections
    for (const auto& client : previousClients_) {
        if (currentClients.find(client) == currentClients.end()) {
            appendLog(QString("Client disconnected: %1").arg(QString::fromStdString(client)), "orange");
        }
    }
    
    // Update previous state
    previousClients_ = currentClients;
    
    if (clients.empty()) {
        QListWidgetItem* noClientsItem = new QListWidgetItem("No clients connected");
        noClientsItem->setForeground(QColor("#999"));
        noClientsItem->setFlags(Qt::NoItemFlags); // Make it non-selectable
        activeClientsWidget->addItem(noClientsItem);
    } else {
        for (const auto& client : clients) {
            activeClientsWidget->addItem(QString("👤 %1").arg(QString::fromStdString(client)));
        }
    }
}

void ServerWindow::onServerLog(const QString& message) {
    // Parse server log messages and add them to transfer log
    QString cleanMsg = message.trimmed();
    
    // Determine color based on message content
    QString color = "black";
    if (cleanMsg.contains("Processing LIST", Qt::CaseInsensitive)) {
        color = "blue";
        appendLog("→ " + cleanMsg, color);
    } else if (cleanMsg.contains("Processing GET", Qt::CaseInsensitive) || 
               cleanMsg.contains("Sending file", Qt::CaseInsensitive)) {
        color = "blue";
        appendLog("→ " + cleanMsg, color);
    } else if (cleanMsg.contains("Processing PUT", Qt::CaseInsensitive) || 
               cleanMsg.contains("Receiving file", Qt::CaseInsensitive)) {
        color = "blue";
        appendLog("→ " + cleanMsg, color);
    } else if (cleanMsg.contains("error", Qt::CaseInsensitive) || 
               cleanMsg.contains("fail", Qt::CaseInsensitive)) {
        color = "red";
        appendLog("✗ " + cleanMsg, color);
    } else if (cleanMsg.contains("success", Qt::CaseInsensitive) || 
               cleanMsg.contains("completed", Qt::CaseInsensitive)) {
        color = "green";
        appendLog("✓ " + cleanMsg, color);
    } else if (cleanMsg.startsWith("[Protocol]") || 
               cleanMsg.startsWith("[Server]") || 
               cleanMsg.startsWith("[Session]")) {
        appendLog(cleanMsg, color);
    }
    // Skip other messages to avoid clutter
}
