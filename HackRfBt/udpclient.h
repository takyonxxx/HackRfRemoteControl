#ifndef UDPCLIENT_H
#define UDPCLIENT_H
#include <QCoreApplication>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QTimer>
#include <QDebug>

class UdpClient : public QObject
{
    Q_OBJECT

public:
    UdpClient(QObject *parent = nullptr) : QObject(parent)
    {
        udpSocket = new QUdpSocket(this);
        serverAddress = QHostAddress::LocalHost;
        serverPort = 5000; // Replace with the server's port
    }

    void setServerAddress(QString &ipAddress)
    {
        QHostAddress _serverAddress(ipAddress);        
        serverAddress = _serverAddress;
        qDebug() << "Client ip: " << serverAddress.toString() << "on port" << serverPort;
    }

    void sendData(QByteArray &data)
    {
        udpSocket->writeDatagram(data, serverAddress, serverPort);
    }

private slots:   
    void readPendingDatagrams()
    {
        while (udpSocket->hasPendingDatagrams())
        {
            QByteArray datagram;
            datagram.resize(udpSocket->pendingDatagramSize());
            QHostAddress senderAddress;
            quint16 senderPort;

            udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress, &senderPort);

            // Handle received data as needed
            qDebug() << "Received data from server: " << datagram;
        }
    }

private:
    QUdpSocket *udpSocket;
    QHostAddress serverAddress;
    quint16 serverPort;
};
#endif // UDPCLIENT_H
