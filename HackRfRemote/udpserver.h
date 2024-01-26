#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QCoreApplication>
#include <QUdpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDebug>

class UdpServer : public QObject
{
    Q_OBJECT

public:
    UdpServer(QObject *parent = nullptr) : QObject(parent)
    {
        udpSocket = new QUdpSocket(this);
        serverPort = 5000;
        auto ipAddress = getServerIpAddress();
        emit sendInfo(ipAddress);

        QHostAddress _serverAddress(ipAddress);
        serverAddress = _serverAddress;

        // Bind the socket to a specific port and address
        if (udpSocket->bind(serverAddress, serverPort))
        {
            qDebug() << "Server bound successfully to" << serverAddress.toString() << "on port" << serverPort;
            connect(udpSocket, &QUdpSocket::readyRead, this, &UdpServer::readPendingDatagrams);
        }
        else
        {
            qDebug() << "Failed to bind server to" << serverAddress.toString() << "on port" << serverPort;
            qDebug() << "Error details:" << udpSocket->errorString();
        }
    }

    QString getServerIpAddress() const
    {
        QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
        foreach (const QNetworkInterface &interface, interfaces)
        {
            if ((interface.name() == "bridge100" || interface.name() == "wlan0" || interface.name() == "ap0" || interface.name() == "rmnet0") &&
                interface.type() == QNetworkInterface::Ethernet && interface.isValid())
            {
                QList<QNetworkAddressEntry> addresses = interface.addressEntries();
                foreach (const QNetworkAddressEntry &address, addresses)
                {
                    if (address.ip().protocol() == QAbstractSocket::IPv4Protocol)
                    {
                        return address.ip().toString();
                    }
                }
            }
        }
        return QString("127.0.0.1");
    }

private slots:

    void readPendingDatagrams()
    {        
        while (udpSocket->hasPendingDatagrams())
        {
            QByteArray datagram;
            datagram.resize(udpSocket->pendingDatagramSize());
            udpSocket->readDatagram(datagram.data(), datagram.size());
            emit sendBuffer(datagram);
        }
    }   

    void sendData(QByteArray &data)
    {
        udpSocket->writeDatagram(data, serverAddress, serverPort);
    }

signals:
    void sendBuffer(QByteArray &buffer);
    void sendInfo(QString &info);
private:
    QUdpSocket *udpSocket;
    QHostAddress serverAddress;
    quint16 serverPort;
};

#endif // UDPSERVER_H
