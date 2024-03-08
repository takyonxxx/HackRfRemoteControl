#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include <QDateTime>
#include <QNetworkInterface>

#include "audiooutput.h"

class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    TcpServer(QObject *parent = nullptr);
    ~TcpServer();

    QString getServerIpAddress();

    void setPtt(bool newPtt);
    void reset();

    void setReadBufferSize(int newReadBufferSize);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();
    void calculateAndEmitAverageBaud(qint64 dataSize, qint64 currentTime);

signals:
    void sendBuffer(QByteArray &buffer);
    void sendInfo(QString);
    void sendBaud(QString);   

private:
    QTcpSocket *clientSocket{};
    AudioOutput *audioOutput{};

    QByteArray partialData;
    qint64 totalReceivedDataSize = 0;
    qint64 totalBaud = 0;
    qint64 numberOfSamples = 0;
    qint64 lastUpdateTime = 0;
    bool m_ptt;
    int readBufferSize;
};

#endif // TCPSERVER_H
