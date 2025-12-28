#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QListWidget>
#include <QLabel>
#include <QTabWidget>
#include <QTimer>
#include "client.h"
#include <memory>

class ClientWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();

private slots:
    void onConnectClicked();
    void onDisconnectClicked();
    void onListFilesClicked();
    void onDownloadFileClicked();
    void onUploadFileClicked();
    void onMetricsClicked();
    void onExportCSVClicked();
    void onQuitClicked();
    void updateUI();
    void onCommandExecute();

private:
    void setupUI();
    void updateStatusBar();
    void updateMetrics();
    void logMessage(const QString& message, bool success = true);
    
    // UI Components
    QLabel *statusLabel;
    QComboBox *commandCombo;
    QPushButton *connectBtn;
    QTabWidget *tabWidget;
    QListWidget *fileListWidget;
    QTextEdit *logTextEdit;
    
    // Metrics labels
    QLabel *bytesSentLabel;
    QLabel *bytesReceivedLabel;
    QLabel *filesUploadedLabel;
    QLabel *filesDownloadedLabel;
    QLabel *avgThroughputSendLabel;
    QLabel *avgThroughputRecvLabel;
    
    // Buttons
    QPushButton *disconnectBtn;
    QPushButton *listFilesBtn;
    QPushButton *downloadBtn;
    QPushButton *uploadBtn;
    QPushButton *metricsBtn;
    QPushButton *exportBtn;
    
    // Core client
    std::unique_ptr<Client> client;
    QTimer *updateTimer;
    
    // State
    bool isConnected;
    QString currentServer;
    quint16 currentPort;
};

#endif // CLIENTWINDOW_H
