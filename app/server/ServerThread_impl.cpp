// ServerThread implementation
#include "ServerWindow.h"
#include <unistd.h>
#include <fcntl.h>
#include <thread>

void ServerThread::run() {
    if (!server_) {
        return;
    }
    
    // Redirect stdout/stderr to capture server logs
    int stdout_pipe[2];
    int stderr_pipe[2];
    
    if (pipe(stdout_pipe) == 0 && pipe(stderr_pipe) == 0) {
        // Make read ends non-blocking
        fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);
        fcntl(stderr_pipe[0], F_SETFL, O_NONBLOCK);
        
        // Save original stdout/stderr
        int stdout_backup = dup(STDOUT_FILENO);
        int stderr_backup = dup(STDERR_FILENO);
        
        // Redirect stdout/stderr to pipes
        dup2(stdout_pipe[1], STDOUT_FILENO);
        dup2(stderr_pipe[1], STDERR_FILENO);
        
        // Close write ends in this thread (they're duplicated to stdout/stderr)
        close(stdout_pipe[1]);
        close(stderr_pipe[1]);
        
        // Run server in separate thread
        std::thread serverRunner([this]() {
            server_->run();
        });
        
        // Read from pipes and emit signals
        char buffer[4096];
        while (!stopRequested_ && server_->isRunning()) {
            // Read from stdout pipe
            ssize_t n = read(stdout_pipe[0], buffer, sizeof(buffer) - 1);
            if (n > 0) {
                buffer[n] = '\0';
                QString message = QString::fromUtf8(buffer);
                // Split by newlines
                QStringList lines = message.split('\n', Qt::SkipEmptyParts);
                for (const QString& line : lines) {
                    if (!line.trimmed().isEmpty()) {
                        emit logMessage(line.trimmed());
                    }
                }
            }
            
            // Read from stderr pipe
            n = read(stderr_pipe[0], buffer, sizeof(buffer) - 1);
            if (n > 0) {
                buffer[n] = '\0';
                QString message = QString::fromUtf8(buffer);
                QStringList lines = message.split('\n', Qt::SkipEmptyParts);
                for (const QString& line : lines) {
                    if (!line.trimmed().isEmpty()) {
                        emit logMessage(line.trimmed());
                    }
                }
            }
            
            // Small delay to avoid busy waiting
            QThread::msleep(100);
        }
        
        // Wait for server thread
        if (serverRunner.joinable()) {
            serverRunner.join();
        }
        
        // Restore stdout/stderr
        dup2(stdout_backup, STDOUT_FILENO);
        dup2(stderr_backup, STDERR_FILENO);
        
        // Close all pipes
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);
        close(stdout_backup);
        close(stderr_backup);
    } else {
        // Fallback: just run server without capturing output
        server_->run();
    }
}
