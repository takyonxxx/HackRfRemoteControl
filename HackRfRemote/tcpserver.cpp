#include "tcpserver.h"


TcpServer::TcpServer(QObject *parent):
     QTcpServer(parent)
{

}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    QTcpSocket *clientSocket = new QTcpSocket(this);
    clientSocket->setSocketDescriptor(socketDescriptor);

    connect(clientSocket, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);
}

void TcpServer::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket && clientSocket->bytesAvailable() > 0) {
        QByteArray data = clientSocket->readAll();
        emit sendBuffer(data);
        // Combine partial data from previous reads, if any
        // data.prepend(partialData);
        // int expectedSize = HackRfManager::getSamplingBytes(currentDemod);
        // while (data.size() >= expectedSize) {
        //     // Process a complete message of expected size
        //     QByteArray message = data.left(expectedSize);
        //     data.remove(0, expectedSize);
        //     emit sendBuffer(message);

        //     qint64 dataSize = message.size();
        //     totalReceivedDataSize += dataSize;
        //     double totalReceivedDataMB = static_cast<double>(totalReceivedDataSize) / (1024 * 1024);
        //     QString totalReceivedDataString = QString::number(totalReceivedDataMB, 'f', 3) + " MB" + " - " + QString::number(dataSize) + " Byte";
        //     emit sendInfo(totalReceivedDataString);
        //     calculateAndEmitAverageBaud(dataSize, QDateTime::currentMSecsSinceEpoch());
        // }
        // // Store any remaining partial data for the next read
        // partialData = data;
    }
}

void TcpServer::calculateAndEmitAverageBaud(qint64 dataSize, qint64 currentTime)
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

void TcpServer::setDemod(HackRfManager::Demod newDemod)
{
    currentDemod = newDemod;
}

QString TcpServer::getServerIpAddress()
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

void TcpServer::reset()
{
    totalReceivedDataSize = 0;
    totalBaud = 0;
    numberOfSamples = 0;
    lastUpdateTime = 0;
}
