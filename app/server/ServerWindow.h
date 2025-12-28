#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QTabWidget>
#include <QTimer>
#include "server.h"
#include <memory>
#include <thread>

class ServerWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private slots:
    void onStartServer();
    void onStopServer();
    void onExecuteCommand();
    void onMetricsClicked();
    void onClientsClicked();
    void onChangeDirClicked();
    void onVerboseClicked();
    void onExportCSVClicked();
    void onQuitClicked();
    void updateUI();

private:
    void setupUI();
    void updateStatusBar();
    void updateMetrics();
    void updateClientList();
    void startServerThread();
    
    // UI Components
    QLabel *statusLabel;
    QLabel *portLabel;
    QLabel *ipLabel;
    QLabel *dirLabel;
    QComboBox *commandCombo;
    QPushButton *executeBtn;
    QTabWidget *tabWidget;
    QListWidget *clientListWidget;
    
    // Metrics labels
    QLabel *activeClientsLabel;
    QLabel *totalConnectionsLabel;
    QLabel *filesSentLabel;
    QLabel *filesReceivedLabel;
    QLabel *dataSentLabel;
    QLabel *dataReceivedLabel;
    
    // Server control
    QLineEdit *portEdit;
    QLineEdit *sharedDirEdit;
    QPushButton *startBtn;
    QPushButton *stopBtn;
    
    // Action buttons
    QPushButton *metricsBtn;
    QPushButton *clientsBtn;
    QPushButton *changeDirBtn;
    QPushButton *verboseBtn;
    QPushButton *exportBtn;
    
    // Core server
    std::unique_ptr<Server> server;
    std::unique_ptr<std::thread> serverThread;
    QTimer *updateTimer;
    
    // State
    bool isRunning;
    quint16 serverPort;
    QString sharedDirectory;
    bool verboseMode;
};

#endif // SERVERWINDOW_H
