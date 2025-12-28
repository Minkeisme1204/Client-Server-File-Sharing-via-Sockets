#include "ServerWindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QProcess>
#include <QThread>

ServerWindow::ServerWindow(QWidget *parent)
    : QMainWindow(parent), server(std::make_unique<Server>()), isRunning(false), 
      serverPort(9000), sharedDirectory("./shared"), verboseMode(true) {
    
    setWindowTitle("File Transfer Server");
    resize(700, 650);
    
    setupUI();
    
    // Update timer
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &ServerWindow::updateUI);
    updateTimer->start(1000); // Update every second
}

ServerWindow::~ServerWindow() {
    updateTimer->stop();
    
    if (isRunning && server && server->isRunning()) {
        server->stop();
        
        // Wait for server thread to finish
        if (serverThread && serverThread->joinable()) {
            serverThread->join();
        }
    }
    
    serverThread.reset();
    server.reset();
}

void ServerWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // ===== Status Bar =====
    QWidget *statusWidget = new QWidget();
    QHBoxLayout *statusLayout = new QHBoxLayout(statusWidget);
    statusLayout->setContentsMargins(10, 5, 10, 5);
    
    statusLabel = new QLabel("Status: Stopped");
    statusLabel->setStyleSheet("QLabel { background-color: #d9534f; color: white; padding: 5px; border-radius: 3px; font-weight: bold; }");
    
    portLabel = new QLabel("Port: -");
    portLabel->setStyleSheet("QLabel { background-color: #5cb85c; color: white; padding: 5px; border-radius: 3px; }");
    
    ipLabel = new QLabel("IP: -");
    ipLabel->setStyleSheet("QLabel { background-color: #0275d8; color: white; padding: 5px; border-radius: 3px; }");
    
    dirLabel = new QLabel("Shared Directory: " + sharedDirectory);
    dirLabel->setStyleSheet("QLabel { background-color: #5cb85c; color: white; padding: 5px; border-radius: 3px; }");
    
    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(portLabel);
    statusLayout->addWidget(ipLabel);
    statusLayout->addWidget(dirLabel);
    statusLayout->addStretch();
    
    mainLayout->addWidget(statusWidget);
    
    // ===== Server Configuration =====
    QGroupBox *configGroup = new QGroupBox("Server Configuration");
    QHBoxLayout *configLayout = new QHBoxLayout();
    
    configLayout->addWidget(new QLabel("Port:"));
    portEdit = new QLineEdit("9000");
    portEdit->setMaximumWidth(80);
    configLayout->addWidget(portEdit);
    
    configLayout->addWidget(new QLabel("Shared Dir:"));
    sharedDirEdit = new QLineEdit("./shared");
    sharedDirEdit->setMinimumWidth(200);
    configLayout->addWidget(sharedDirEdit);
    
    startBtn = new QPushButton("Start Server");
    startBtn->setStyleSheet("QPushButton { background-color: #5cb85c; color: white; padding: 5px 15px; font-weight: bold; } QPushButton:hover { background-color: #449d44; }");
    connect(startBtn, &QPushButton::clicked, this, &ServerWindow::onStartServer);
    configLayout->addWidget(startBtn);
    
    stopBtn = new QPushButton("Stop Server");
    stopBtn->setStyleSheet("QPushButton { background-color: #d9534f; color: white; padding: 5px 15px; } QPushButton:disabled { background-color: #cccccc; }");
    stopBtn->setEnabled(false);
    connect(stopBtn, &QPushButton::clicked, this, &ServerWindow::onStopServer);
    configLayout->addWidget(stopBtn);
    
    configGroup->setLayout(configLayout);
    mainLayout->addWidget(configGroup);
    
    // ===== Command Section =====
    QWidget *commandWidget = new QWidget();
    QHBoxLayout *commandLayout = new QHBoxLayout(commandWidget);
    
    QLabel *cmdLabel = new QLabel("Command:");
    commandCombo = new QComboBox();
    commandCombo->setEditable(true);
    commandCombo->addItems({
        "clients",
        "metrics",
        "verbose"
    });
    commandCombo->setMinimumWidth(300);
    
    executeBtn = new QPushButton("Execute");
    executeBtn->setStyleSheet("QPushButton { background-color: #0275d8; color: white; padding: 5px 15px; font-weight: bold; } QPushButton:disabled { background-color: #cccccc; }");
    executeBtn->setEnabled(false);
    connect(executeBtn, &QPushButton::clicked, this, &ServerWindow::onExecuteCommand);
    
    commandLayout->addWidget(cmdLabel);
    commandLayout->addWidget(commandCombo);
    commandLayout->addWidget(executeBtn);
    
    mainLayout->addWidget(commandWidget);
    
    // ===== Tab Widget =====
    tabWidget = new QTabWidget();
    
    // Active Clients Tab
    clientListWidget = new QListWidget();
    clientListWidget->setAlternatingRowColors(true);
    tabWidget->addTab(clientListWidget, "Active Clients");
    
    // Server Metrics Tab
    QWidget *metricsWidget = new QWidget();
    QVBoxLayout *metricsTabLayout = new QVBoxLayout(metricsWidget);
    
    QGroupBox *metricsGroup = new QGroupBox("Server Statistics");
    QGridLayout *metricsLayout = new QGridLayout();
    
    metricsLayout->addWidget(new QLabel("Active Clients:"), 0, 0);
    activeClientsLabel = new QLabel("0");
    activeClientsLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 16px; }");
    metricsLayout->addWidget(activeClientsLabel, 0, 1);
    
    metricsLayout->addWidget(new QLabel("Total Connections:"), 1, 0);
    totalConnectionsLabel = new QLabel("0");
    totalConnectionsLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 16px; }");
    metricsLayout->addWidget(totalConnectionsLabel, 1, 1);
    
    metricsLayout->addWidget(new QLabel("Files Sent:"), 2, 0);
    filesSentLabel = new QLabel("0");
    filesSentLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 16px; }");
    metricsLayout->addWidget(filesSentLabel, 2, 1);
    
    metricsLayout->addWidget(new QLabel("Files Received:"), 3, 0);
    filesReceivedLabel = new QLabel("0");
    filesReceivedLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 16px; }");
    metricsLayout->addWidget(filesReceivedLabel, 3, 1);
    
    metricsLayout->addWidget(new QLabel("Total Data Sent:"), 4, 0);
    dataSentLabel = new QLabel("0 MB");
    dataSentLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 16px; }");
    metricsLayout->addWidget(dataSentLabel, 4, 1);
    
    metricsLayout->addWidget(new QLabel("Total Data Received:"), 5, 0);
    dataReceivedLabel = new QLabel("0 MB");
    dataReceivedLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 16px; }");
    metricsLayout->addWidget(dataReceivedLabel, 5, 1);
    
    metricsGroup->setLayout(metricsLayout);
    metricsTabLayout->addWidget(metricsGroup);
    metricsTabLayout->addStretch();
    
    tabWidget->addTab(metricsWidget, "Server Metrics");
    
    mainLayout->addWidget(tabWidget);
    
    // ===== Action Buttons =====
    QWidget *btnWidget = new QWidget();
    QHBoxLayout *btnLayout = new QHBoxLayout(btnWidget);
    
    metricsBtn = new QPushButton("Metrics");
    metricsBtn->setStyleSheet("QPushButton { background-color: #0275d8; color: white; }");
    connect(metricsBtn, &QPushButton::clicked, this, &ServerWindow::onMetricsClicked);
    
    clientsBtn = new QPushButton("Clients");
    clientsBtn->setStyleSheet("QPushButton { background-color: #5bc0de; color: white; }");
    connect(clientsBtn, &QPushButton::clicked, this, &ServerWindow::onClientsClicked);
    
    changeDirBtn = new QPushButton("Change Dir");
    changeDirBtn->setStyleSheet("QPushButton { background-color: #5cb85c; color: white; }");
    changeDirBtn->setEnabled(false);
    connect(changeDirBtn, &QPushButton::clicked, this, &ServerWindow::onChangeDirClicked);
    
    verboseBtn = new QPushButton("Verbose");
    verboseBtn->setStyleSheet("QPushButton { background-color: #f0ad4e; color: white; }");
    connect(verboseBtn, &QPushButton::clicked, this, &ServerWindow::onVerboseClicked);
    
    exportBtn = new QPushButton("Export CSV");
    exportBtn->setStyleSheet("QPushButton { background-color: #5bc0de; color: white; }");
    connect(exportBtn, &QPushButton::clicked, this, &ServerWindow::onExportCSVClicked);
    
    btnLayout->addWidget(metricsBtn);
    btnLayout->addWidget(clientsBtn);
    btnLayout->addWidget(changeDirBtn);
    btnLayout->addWidget(verboseBtn);
    btnLayout->addWidget(exportBtn);
    
    mainLayout->addWidget(btnWidget);
    
    // ===== Quit Button =====
    QPushButton *quitBtn = new QPushButton("Quit");
    quitBtn->setStyleSheet("QPushButton { background-color: #d9534f; color: white; padding: 8px; font-weight: bold; }");
    quitBtn->setMaximumWidth(150);
    connect(quitBtn, &QPushButton::clicked, this, &ServerWindow::onQuitClicked);
    
    QHBoxLayout *quitLayout = new QHBoxLayout();
    quitLayout->addStretch();
    quitLayout->addWidget(quitBtn);
    
    mainLayout->addLayout(quitLayout);
}

void ServerWindow::onStartServer() {
    bool ok;
    quint16 port = portEdit->text().toUShort(&ok);
    
    if (!ok || port == 0) {
        QMessageBox::warning(this, "Invalid Port", "Please enter a valid port number");
        return;
    }
    
    QString sharedDir = sharedDirEdit->text();
    if (sharedDir.isEmpty()) {
        QMessageBox::warning(this, "Invalid Directory", "Please enter a valid shared directory");
        return;
    }
    
    serverPort = port;
    sharedDirectory = sharedDir;
    
    server->setVerbose(verboseMode);
    server->setMaxConnections(10);
    
    if (server->start(serverPort, sharedDirectory.toStdString())) {
        isRunning = true;
        
        // Start server thread
        startServerThread();
        
        // Update UI
        startBtn->setEnabled(false);
        stopBtn->setEnabled(true);
        executeBtn->setEnabled(true);
        changeDirBtn->setEnabled(true);
        portEdit->setEnabled(false);
        sharedDirEdit->setEnabled(false);
        
        updateStatusBar();
        
        QMessageBox::information(this, "Success", "Server started on port " + QString::number(serverPort));
    } else {
        QMessageBox::critical(this, "Error", "Failed to start server. Port may be in use.");
    }
}

void ServerWindow::onStopServer() {
    if (server && server->isRunning()) {
        // First update UI to show stopping state
        stopBtn->setEnabled(false);
        stopBtn->setText("Stopping...");
        
        // Stop the server (this will stop accept loop)
        server->stop();
        
        // Wait for server thread to finish
        if (serverThread->joinable()) {
            serverThread->join();
        }
        serverThread.reset();
        
        isRunning = false;
        
        // Update UI
        startBtn->setEnabled(true);
        stopBtn->setEnabled(false);
        stopBtn->setText("Stop Server");
        executeBtn->setEnabled(false);
        changeDirBtn->setEnabled(false);
        portEdit->setEnabled(true);
        sharedDirEdit->setEnabled(true);
        
        clientListWidget->clear();
        updateStatusBar();
        
        QMessageBox::information(this, "Stopped", "Server stopped successfully");
    }
}

void ServerWindow::onExecuteCommand() {
    QString command = commandCombo->currentText().toLower();
    
    if (command == "clients") {
        onClientsClicked();
    } else if (command == "metrics") {
        onMetricsClicked();
    } else if (command == "verbose") {
        onVerboseClicked();
    }
}

void ServerWindow::onMetricsClicked() {
    updateMetrics();
    tabWidget->setCurrentIndex(1); // Switch to metrics tab
}

void ServerWindow::onClientsClicked() {
    updateClientList();
    tabWidget->setCurrentIndex(0); // Switch to clients tab
}

void ServerWindow::onChangeDirClicked() {
    QString newDir = QFileDialog::getExistingDirectory(this, "Select Shared Directory", sharedDirectory);
    if (!newDir.isEmpty()) {
        if (server->setSharedDirectory(newDir.toStdString())) {
            sharedDirectory = newDir;
            sharedDirEdit->setText(newDir);
            dirLabel->setText("Shared Directory: " + newDir);
            QMessageBox::information(this, "Success", "Shared directory changed to: " + newDir);
        } else {
            QMessageBox::critical(this, "Error", "Failed to change directory");
        }
    }
}

void ServerWindow::onVerboseClicked() {
    verboseMode = !verboseMode;
    server->setVerbose(verboseMode);
    
    QString status = verboseMode ? "ON" : "OFF";
    QMessageBox::information(this, "Verbose Mode", "Verbose logging: " + status);
}

void ServerWindow::onExportCSVClicked() {
    QString filename = QFileDialog::getSaveFileName(this, "Export Metrics", "server_metrics.csv", "CSV Files (*.csv)");
    if (filename.isEmpty()) {
        return;
    }
    
    if (server->exportMetrics(filename.toStdString())) {
        QMessageBox::information(this, "Success", "Metrics exported to " + filename);
    } else {
        QMessageBox::critical(this, "Error", "Failed to export metrics");
    }
}

void ServerWindow::onQuitClicked() {
    if (server && server->isRunning()) {
        server->stop();
        if (serverThread && serverThread->joinable()) {
            serverThread->join();
        }
    }
    QApplication::quit();
}

void ServerWindow::updateUI() {
    if (isRunning) {
        updateMetrics();
        updateClientList();
    }
}

void ServerWindow::updateStatusBar() {
    if (isRunning) {
        statusLabel->setText("Status: Running");
        statusLabel->setStyleSheet("QLabel { background-color: #5cb85c; color: white; padding: 5px; border-radius: 3px; font-weight: bold; }");
        portLabel->setText("Listening on Port " + QString::number(serverPort));
        
        // Get server IP
        QProcess process;
        process.start("hostname", QStringList() << "-I");
        process.waitForFinished();
        QString ip = process.readAllStandardOutput().trimmed().split(' ').first();
        if (ip.isEmpty()) ip = "127.0.0.1";
        ipLabel->setText("IP: " + ip);
    } else {
        statusLabel->setText("Status: Stopped");
        statusLabel->setStyleSheet("QLabel { background-color: #d9534f; color: white; padding: 5px; border-radius: 3px; font-weight: bold; }");
        portLabel->setText("Port: -");
        ipLabel->setText("IP: -");
    }
}

void ServerWindow::updateMetrics() {
    if (!server) return;
    
    const auto& metrics = server->getMetrics();
    
    activeClientsLabel->setText(QString::number(server->getActiveSessionCount()));
    totalConnectionsLabel->setText(QString::number(metrics.totalConnections.load()));
    filesSentLabel->setText(QString::number(metrics.filesDownloaded.load()));
    filesReceivedLabel->setText(QString::number(metrics.filesUploaded.load()));
    dataSentLabel->setText(QString::number(metrics.totalBytesSent.load() / 1024.0 / 1024.0, 'f', 1) + " MB");
    dataReceivedLabel->setText(QString::number(metrics.totalBytesReceived.load() / 1024.0 / 1024.0, 'f', 1) + " MB");
}

void ServerWindow::updateClientList() {
    if (!server) return;
    
    clientListWidget->clear();
    
    auto clients = server->getActiveClients();
    for (const auto& client : clients) {
        clientListWidget->addItem("👤 " + QString::fromStdString(client));
    }
    
    if (clients.empty()) {
        clientListWidget->addItem("No active clients");
    }
}

void ServerWindow::startServerThread() {
    serverThread = std::make_unique<std::thread>([this]() {
        server->run();
    });
    // Keep thread joinable for proper cleanup
}
