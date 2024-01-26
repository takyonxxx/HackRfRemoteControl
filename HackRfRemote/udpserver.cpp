#include "udpserver.h"

UdpServer::UdpServer(QObject *parent):
    QObject(parent)
{
    udpSocket = new QUdpSocket(this);
    serverPort = 5000;

    auto ipAddress = getServerIpAddress();
    QHostAddress _serverAddress(ipAddress);
    serverAddress = _serverAddress;

    // Bind the audio socket to a specific port and address
    if (udpSocket->bind(serverAddress, serverPort))
    {
        qDebug() << "Server audio bound successfully to" << serverAddress.toString() << "on port" << serverPort;
        connect(udpSocket, &QUdpSocket::readyRead, this, &UdpServer::readPendingDatagrams);
    }
    else
    {
        qDebug() << "Failed to bind audio server to" << serverAddress.toString() << "on port" << serverPort;
        qDebug() << "Error details:" << udpSocket->errorString();
    }
}

QString UdpServer::getServerIpAddress()
{
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    foreach (const QNetworkInterface &interface, interfaces)
    {
        if ((interface.name() == "en0"
             || interface.name() == "bridge100"
             || interface.name() == "wlan0"
             || interface.name() == "ap0"
             || interface.name() == "rmnet0") &&
            interface.isValid())
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

void UdpServer::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        emit sendBuffer(datagram);

        qint64 dataSize = datagram.size();
        totalReceivedDataSize += dataSize;
        double totalReceivedDataMB = static_cast<double>(totalReceivedDataSize) / (1024 * 1024);
        QString totalReceivedDataString = QString::number(totalReceivedDataMB, 'f', 3) + " MB" + " - " + QString::number(dataSize) + " Byte";
        emit sendInfo(totalReceivedDataString);
        calculateAndEmitAverageBaud(dataSize, QDateTime::currentMSecsSinceEpoch());
    }
}

void UdpServer::sendData(QByteArray &data)
{
    udpSocket->writeDatagram(data, serverAddress, serverPort);
}

void UdpServer::calculateAndEmitAverageBaud(qint64 dataSize, qint64 currentTime)
{
    // Calculate time difference since the last update
    qint64 deltaTime = currentTime - lastUpdateTime;

    if (deltaTime > 0)
    {
        // Calculate baud for the current update
        qint64 bitsTransferred = dataSize * 8;
        qint64 baud = (bitsTransferred * 1000) / deltaTime;  // Assuming time is in milliseconds

        // Update total baud and number of samples
        totalBaud += baud;
        numberOfSamples++;

        // Calculate average baud rate in kbaud
        double averageBaudK = static_cast<double>(totalBaud) / numberOfSamples / 1000;

        // Emit the average baud rate in kbaud
        emit sendBaud(QString("Baud: %1 kbaud").arg(averageBaudK, 0, 'f', 1));
    }

    // Update lastUpdateTime for the next calculation
    lastUpdateTime = currentTime;
}

void UdpServer::reset()
{
    totalReceivedDataSize = 0;
    totalBaud = 0;
    numberOfSamples = 0;
    lastUpdateTime = 0;
}
