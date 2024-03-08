#include "tcpserver.h"
#include "constants.h"

TcpServer::TcpServer(QObject *parent):
     QTcpServer(parent)
{
    readBufferSize = 50;
    audioOutput = new AudioOutput(this, DEFAULT_SAMPLE_RATE);
}

TcpServer::~TcpServer()
{
}

void TcpServer::setPtt(bool newPtt)
{
    m_ptt = newPtt;
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
    clientSocket = new QTcpSocket(this);
    clientSocket->setSocketDescriptor(socketDescriptor);
    clientSocket->setReadBufferSize(1024*readBufferSize);

    connect(clientSocket, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);
}

void TcpServer::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket && clientSocket->bytesAvailable() > 0) {
        qint64 dataSize = clientSocket->bytesAvailable();
        QByteArray data = clientSocket->read(dataSize);
        if (!m_ptt && audioOutput && !data.isEmpty()) {
            audioOutput->writeBuffer(data);
            totalReceivedDataSize += dataSize;
            double totalReceivedDataMB = static_cast<double>(totalReceivedDataSize) / (1024 * 1024);
            QString totalReceivedDataString = QString::number(totalReceivedDataMB, 'f', 1) + " MB" + " - " + QString::number(dataSize) + " Byte";
            emit sendInfo(totalReceivedDataString);
            calculateAndEmitAverageBaud(dataSize, QDateTime::currentMSecsSinceEpoch());
        }
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

void TcpServer::setReadBufferSize(int newReadBufferSize)
{
    readBufferSize = newReadBufferSize;
    clientSocket->setReadBufferSize(1024*readBufferSize);
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
    m_ptt = false;
}
