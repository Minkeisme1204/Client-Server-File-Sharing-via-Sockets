#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QTimer>
#include <QThread>
#include <QComboBox>
#include <QTabWidget>
#include <QListWidget>
#include <QDateTime>
#include <memory>
#include <set>
#include "../../include/server.h"

class ServerThread : public QThread {
    Q_OBJECT
public:
    explicit ServerThread(Server* server, QObject* parent = nullptr)
        : QThread(parent), server_(server), stopRequested_(false) {}
    
    void run() override;
    void requestStop() { stopRequested_ = true; }
    
signals:
    void logMessage(const QString& message);
    
private:
    Server* server_;
    std::atomic<bool> stopRequested_;
};

class ServerWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private slots:
    void onExecuteCommand();
    void onMetricsClicked();
    void onClientsClicked();
    void onChangeDirClicked();
    void onVerboseClicked();
    void onExportClicked();
    void onStopServerClicked();
    void onQuitClicked();
    void updateStatus();
    void updateMetrics();
    void onServerLog(const QString& message);

private:
    void setupUI();
    void processCommand(const QString& command);
    void startServer(int port, const QString& sharedDir);
    void stopServer();
    void updateClientsList();
    void updateStatusBar();
    void appendLog(const QString& text, const QString& color = "black");
    QString getServerIP();
    QString getTailscaleIP();
    
    // UI Components
    QLabel* statusLabel;
    QLabel* connectionLabel;
    QComboBox* commandCombo;
    QPushButton* executeButton;
    
    // Tab widget
    QTabWidget* tabWidget;
    QListWidget* activeClientsWidget;
    QTextEdit* transferLogText;
    
    // Metrics labels
    QLabel* activeClientsCountLabel;
    QLabel* totalConnectionsLabel;
    QLabel* filesSentLabel;
    QLabel* filesReceivedLabel;
    QLabel* dataSentLabel;
    QLabel* dataReceivedLabel;
    QLabel* packetLossLabel;
    QLabel* avgThroughputLabel;
    QLabel* peakThroughputLabel;
    QLabel* avgLatencyLabel;
    QLabel* serverIPLabel;
    QLabel* uptimeLabel;
    
    // Buttons
    QPushButton* metricsButton;
    QPushButton* clientsButton;
    QPushButton* changeDirButton;
    QPushButton* verboseButton;
    QPushButton* exportButton;
    QPushButton* stopServerButton;
    QPushButton* quitButton;
    
    // Server instance and thread
    std::unique_ptr<Server> server;
    ServerThread* serverThread;
    QTimer* statusTimer;
    
    // Server info
    int currentPort;
    QString currentSharedDir;
    bool verboseMode;
    QDateTime serverStartTime;
    
    // Track previous state for change detection
    std::set<std::string> previousClients_;
    uint64_t previousFilesSent_;
    uint64_t previousFilesReceived_;
    uint64_t previousTotalConnections_;
};

#endif // SERVERWINDOW_H
