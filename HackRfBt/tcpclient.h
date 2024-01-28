#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    TcpClient(QObject *parent = nullptr) : QObject(parent)
    {
        socket = new QTcpSocket(this);
        connect(socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
        connect(socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    }

public slots:
    void connectToServer(const QString &hostAddress, quint16 port)
    {
        socket->connectToHost(QHostAddress(hostAddress), port);
    }

    void sendData(const QByteArray &data)
    {
        if (socket->state() == QAbstractSocket::ConnectedState) {
            socket->write(data);
            socket->flush();
        }
    }

private slots:
    void onConnected()
    {
        qDebug() << "Connected to remote server";
    }

    void onDisconnected()
    {
        qDebug() << "Disconnected from remote server";
    }

private:
    QTcpSocket *socket;
};

#endif // TCPCLIENT_H
