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
        udpAudioSocket = new QUdpSocket(this);
        udpDataSocket = new QUdpSocket(this);
        serverAddress = QHostAddress::LocalHost;
        serverAudioPort = 5000;
        serverDataPort = 5001;
    }

    void setServerAddress(QString &ipAddress)
    {
        QHostAddress _serverAddress(ipAddress);        
        serverAddress = _serverAddress;
        qDebug() << "Client ip: " << serverAddress.toString() << "auido port" << serverAudioPort << "fft port" << serverDataPort;
    }

    void sendAudioData(QByteArray &data)
    {
        udpAudioSocket->writeDatagram(data, serverAddress, serverAudioPort);
    }

    void sendFftData(QByteArray &data)
    {
        udpDataSocket->writeDatagram(data, serverAddress, serverDataPort);
    }

private:
    QUdpSocket *udpAudioSocket;
    QUdpSocket *udpDataSocket;
    QHostAddress serverAddress;
    quint16 serverAudioPort;
    quint16 serverDataPort;
};
#endif // UDPCLIENT_H
