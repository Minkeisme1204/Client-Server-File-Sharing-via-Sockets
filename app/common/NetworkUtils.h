#ifndef NETWORKUTILS_H
#define NETWORKUTILS_H

#include <QString>
#include <QProcess>
#include <QNetworkInterface>
#include <QHostAddress>

class NetworkUtils {
public:
    // Get Tailscale IPv4 address
    static QString getTailscaleIP() {
        // Method 1: Try using tailscale command
        QProcess process;
        process.start("tailscale", QStringList() << "ip" << "-4");
        
        if (process.waitForFinished(2000)) { // 2 second timeout
            QString output = QString::fromUtf8(process.readAllStandardOutput()).trimmed();
            if (!output.isEmpty() && isValidIPv4(output)) {
                return output;
            }
        }
        
        // Method 2: Try to find tailscale0 interface
        foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
            if (interface.name().contains("tailscale", Qt::CaseInsensitive)) {
                foreach (const QNetworkAddressEntry &entry, interface.addressEntries()) {
                    QHostAddress addr = entry.ip();
                    if (addr.protocol() == QAbstractSocket::IPv4Protocol && 
                        !addr.isLoopback()) {
                        return addr.toString();
                    }
                }
            }
        }
        
        // Method 3: Look for IP in 100.x.x.x range (Tailscale CGNAT range)
        foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && 
                !address.isLoopback()) {
                QString ip = address.toString();
                if (ip.startsWith("100.")) {
                    return ip;
                }
            }
        }
        
        return QString(); // Return empty if not found
    }
    
    // Check if Tailscale is installed and running
    static bool isTailscaleAvailable() {
        QProcess process;
        process.start("tailscale", QStringList() << "status");
        return process.waitForFinished(2000) && process.exitCode() == 0;
    }
    
    // Get all network IPs (for display purposes)
    static QStringList getAllNetworkIPs() {
        QStringList ips;
        foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && 
                !address.isLoopback()) {
                ips << address.toString();
            }
        }
        return ips;
    }
    
    // Get local IP (non-Tailscale)
    static QString getLocalIP() {
        foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
            if (address.protocol() == QAbstractSocket::IPv4Protocol && 
                !address.isLoopback() && 
                !address.toString().startsWith("100.")) {
                return address.toString();
            }
        }
        return "127.0.0.1";
    }

private:
    static bool isValidIPv4(const QString& ip) {
        QHostAddress addr(ip);
        return !addr.isNull() && addr.protocol() == QAbstractSocket::IPv4Protocol;
    }
};

#endif // NETWORKUTILS_H
