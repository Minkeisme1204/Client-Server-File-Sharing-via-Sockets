#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

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
#include <QComboBox>
#include <QTabWidget>
#include <QListWidget>
#include <QDateTime>
#include <memory>
#include "../../include/client.h"

class ClientWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void onExecuteCommand();
    void onConnectClicked();
    void onDisconnectClicked();
    void onListFilesClicked();
    void onDownloadClicked();
    void onUploadClicked();
    void onResetMetricsClicked();
    void onExportClicked();
    void onQuitClicked();
    void updateStatus();
    void updateMetrics();

private:
    void setupUI();
    void processCommand(const QString& command);
    void appendLog(const QString& text, bool success = true);
    void connectToServer(const QString& ip, int port);
    void updateStatusBar();
    void parseFileListResponse(const std::string& response);
    
    // UI Components
    QLabel* statusLabel;
    QLabel* connectionLabel;
    QComboBox* commandCombo;
    QPushButton* executeButton;
    
    // Tab widget
    QTabWidget* tabWidget;
    QListWidget* fileListWidget;
    QTextEdit* transferLogText;
    
    // Metrics labels
    QLabel* bytesSentLabel;
    QLabel* bytesReceivedLabel;
    QLabel* filesUploadedLabel;
    QLabel* filesDownloadedLabel;
    QLabel* avgThroughputSentLabel;
    QLabel* avgThroughputRecvLabel;
    QLabel* packetLossLabel;
    QLabel* serverInfoLabel;
    QLabel* uptimeLabel;
    
    // Buttons
    QPushButton* disconnectButton;
    QPushButton* listFilesButton;
    QPushButton* downloadButton;
    QPushButton* uploadButton;
    QPushButton* metricsButton;
    QPushButton* exportButton;
    QPushButton* quitButton;
    
    // Client instance
    std::unique_ptr<Client> client;
    QTimer* statusTimer;
    
    // Connection info
    QString currentIP;
    int currentPort;
    QDateTime connectionStartTime;
};

#endif // CLIENTWINDOW_H
