#ifndef UDPCLIENT_H
#define UDPCLIENT_H
#include <QCoreApplication>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QDebug>

class UdpClient : public QObject
{
    Q_OBJECT

public:
    UdpClient(QObject *parent = nullptr) : QObject(parent)
    {
        udpSocket = new QUdpSocket(this);
        serverAddress = QHostAddress::LocalHost;
        serverPort = 5000;
    }

    void setServerAddress(QString &ipAddress)
    {
        QHostAddress _serverAddress(ipAddress);        
        serverAddress = _serverAddress;
        qDebug() << "Client ip: " << serverAddress.toString() << "auido port" << serverPort;
    }

    void sendData(QByteArray &data)
    {
        udpSocket->writeDatagram(data, serverAddress, serverPort);
    }   

private:
    QUdpSocket *udpSocket;
    QHostAddress serverAddress;
    quint16 serverPort;
};
#endif // UDPCLIENT_H
