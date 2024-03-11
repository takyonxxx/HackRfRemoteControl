#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QThread>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    TcpClient(QObject *parent = nullptr) : QObject(parent), socket(nullptr)
    {
        QMetaObject::invokeMethod(this, &TcpClient::initialize);
    }

public:
    void initialize()
    {
        // Create the socket in the current thread
        socket = new QTcpSocket(this);
        connect(socket, &QTcpSocket::connected, this, &TcpClient::onConnected);
        connect(socket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    }

    void connectToServer(const QString &hostAddress, quint16 port)
    {
        _hostAddress = hostAddress;
        _port = port;

        // Connect to the server
        QMetaObject::invokeMethod(this, [this, hostAddress, port]() {
            socket->connectToHost(QHostAddress(hostAddress), port);
        });
    }

    void sendData(const QByteArray &data)
    {

        QMetaObject::invokeMethod(this, [this, data]() {
            if (socket && socket->state() == QAbstractSocket::ConnectedState) {
                socket->write(data);
                socket->flush();
            }
        });
    }

    bool isSocketCreated() const
    {
        return (socket != nullptr);
    }

private slots:
    void onConnected()
    {
        qDebug() << "Connected to remote server : " << _hostAddress << "port : " << _port;
    }

    void onDisconnected()
    {
        qDebug() << "Disconnected from remote server";
    }

private:
    QTcpSocket *socket;
    QString _hostAddress;
    int _port;
};


#endif // TCPCLIENT_H
