#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QCoreApplication>
#include <QUdpSocket>
#include <QTimer>
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
    void readPendingAudioDatagrams();
    void readPendingDataDatagrams();
//    void sendData(QByteArray &data);
    void calculateAndEmitAverageBaud(qint64 dataSize, qint64 currentTime);

signals:
    void sendAudioBuffer(QByteArray &buffer);
    void sendDataBuffer(QByteArray &buffer);
    void sendInfo(QString);
    void sendBaud(QString);
private:
    QUdpSocket *udpSocketAudio;
    QUdpSocket *udpSocketData;
    QHostAddress serverAddress;
    quint16 serverPortAudio;
    quint16 serverPortData;
    qint64 totalReceivedDataSize = 0;
    qint64 totalBaud = 0;
    qint64 numberOfSamples = 0;
    qint64 lastUpdateTime = 0;
};

#endif // UDPSERVER_H
