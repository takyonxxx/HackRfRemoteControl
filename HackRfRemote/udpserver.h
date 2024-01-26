#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QCoreApplication>
#include <QUdpSocket>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QDateTime>
#include <QDebug>

class UdpServer : public QObject
{
    Q_OBJECT

public:
    UdpServer(QObject *parent = nullptr);
    QString getServerIpAddress();
    void reset();

private slots:
    void readPendingDatagrams();
    void sendData(QByteArray &data);
    void calculateAndEmitAverageBaud(qint64 dataSize, qint64 currentTime);

signals:
    void sendBuffer(QByteArray &buffer);
    void sendInfo(QString);
    void sendBaud(QString);
private:
    QUdpSocket *udpSocket;
    QHostAddress serverAddress;
    quint16 serverPort;
    qint64 totalReceivedDataSize = 0;
    qint64 totalBaud = 0;
    qint64 numberOfSamples = 0;
    qint64 lastUpdateTime = 0;
};

#endif // UDPSERVER_H
