#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "freqctrl.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), m_connected(false),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initButtons();
    setWindowTitle(tr("BalanceRobot Remote Control"));

    ui->m_textStatus->setStyleSheet("font-size: 12pt; color: #cccccc; background-color: #003333;");
    ui->labelDataReceived->setStyleSheet("font-size: 24pt; color: #ffffff; background-color: #250A04;");
    ui->labelBaud->setStyleSheet("font-size: 24pt; color: #ffffff; background-color: #250A04;");

    ui->m_pBConnect->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #097532;");
    ui->m_pReset->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #097532;");
    ui->m_pBExit->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #097532;");
    ui->m_pBSpeak->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_pBSetFreq->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_cFreqType->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_cDemod->setStyleSheet("font-size: 18pt; font: bold; color: #ffffff; background-color: #900C3F;");
    ui->m_lEditFreq->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #900C3F;");

    ui->m_pIncFreq->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #0E8092;");
    ui->m_pDecFreq->setStyleSheet("font-size: 24pt; font: bold; color: #ffffff; background-color: #0E8092;");

    ui->freqCtrl->setG_constant(2.5);
    ui->freqCtrl->Setup(11, 0, 2200e6, 1, UNITS_MHZ);
    ui->freqCtrl->SetDigitColor(QColor("#FFC300"));
    ui->freqCtrl->SetFrequency(DEFAULT_FREQUENCY);

    currentDemod    = HackRfManager::Demod::DEMOD_WFM;
    currentFreqMod  = HackRfManager::FreqMod::MHZ;

    ui->m_cFreqType->setCurrentIndex(2);
    ui->m_cDemod->setCurrentIndex(1);

    m_bleConnection = new BluetoothClient();

    connect(m_bleConnection, &BluetoothClient::statusChanged, this, &MainWindow::getInfo);
    connect(m_bleConnection, &BluetoothClient::changedState, this, &MainWindow::changedState);
    connect(m_bleConnection, &BluetoothClient::sendInfo, this, &MainWindow::getDataReceived);
    connect(m_bleConnection, &BluetoothClient::sendBaud, this, &MainWindow::getBaud);
    connect(m_bleConnection, &BluetoothClient::newData, this, &MainWindow::DataHandler);    

    hackRfManager = new HackRfManager(this);
    connect(hackRfManager, &HackRfManager::sendInfo, this, &MainWindow::getDataReceived);
    hackRfManager->setDemod(currentDemod);
    hackRfManager->start();    

    connect(ui->m_pBConnect, SIGNAL(clicked()),this, SLOT(on_ConnectClicked()));
    connect(ui->m_pBExit, SIGNAL(clicked()),this, SLOT(on_Exit()));
}

MainWindow::~MainWindow()
{
    if (hackRfManager) {
        hackRfManager->setStop(true);
        hackRfManager->wait();
        delete hackRfManager;
    }
    if (udpServer) {
    delete udpServer;
    }

    if (tcpServer) {
    delete tcpServer;
    }

    delete ui;
}

void MainWindow::changedState(BluetoothClient::bluetoothleState state){

    switch(state){

    case BluetoothClient::Scanning:
    {
        getInfo("Searching for low energy devices...");
        break;
    }
    case BluetoothClient::ScanFinished:
    {
        break;
    }

    case BluetoothClient::Connecting:
    {
        break;
    }
    case BluetoothClient::Connected:
    {
        ui->m_pBSpeak->setEnabled(true);
        ui->m_pBConnect->setText("Disconnect");
        m_connected = true;

        tcpServer = new TcpServer(this);
        if (!tcpServer->listen(QHostAddress::Any, 5001)) {
            qDebug() << "Tcp Server could not start. " << tcpServer->errorString();
        }
        connect(tcpServer, &TcpServer::sendBuffer, this, &MainWindow::getBuffer);
        connect(tcpServer, &TcpServer::sendInfo, this, &MainWindow::getDataReceived);
        connect(tcpServer, &TcpServer::sendBaud, this, &MainWindow::getBaud);
        tcpServer->setDemod(currentDemod);

        qDebug() << "Tcp Server listening on port 5001";
        getInfo("Tcp Server listening on port 5001");
        break;
    }
    case BluetoothClient::DisConnected:
    {
        m_connected = false;
        ui->m_pBConnect->setEnabled(true);
        ui->m_pBConnect->setText("Connect");
        ui->m_pBSpeak->setEnabled(false);
        if (tcpServer) {
            tcpServer->disconnect();
            tcpServer->deleteLater();
            tcpServer = nullptr;
        }
        getInfo("Device disconnected.");
        break;
    }
    case BluetoothClient::ServiceFound:
    {
        break;
    }
    case BluetoothClient::AcquireData:
    {
        setIp();

        requestData(mGetFreq);
        requestData(mGetFreqMod);
        requestData(mGetDeMod);
        requestData(mGetPtt);

        break;
    }
    case BluetoothClient::Error:
    {
        ui->m_textStatus->clear();
        break;
    }
    default:
        //nothing for now
        break;
    }
}

void MainWindow::getDataReceived(const QString &info)
{
    ui->labelDataReceived->setText(info);
}

void MainWindow::getBaud(const QString &info)
{
    ui->labelBaud->setText(info);
}

void MainWindow::DataHandler(QByteArray data)
{
    uint8_t parsedCommand;
    uint8_t rw;
    QByteArray parsedValue;
    parseMessage(&data, parsedCommand, parsedValue, rw);
    bool ok;
    int value =  parsedValue.toHex().toInt(&ok, 16);

    if (rw == mWrite)
    {
        switch (parsedCommand)
        {
        case mData:
            {
                auto strValue = QString(parsedValue.toStdString().c_str());
                QStringList parts = strValue.split(',');
                // Check if the received data has at least two parts (command and value)
                if (parts.size() == 2) {
                    QString command = parts[0];
                    QString valueStr = parts[1].trimmed();

                    if (command == "get_freq") {
                        bool conversionOK;
                        double frequencyDouble = valueStr.toDouble(&conversionOK);

                        if (conversionOK) {
                            qint64 frequency = static_cast<qint64>(frequencyDouble);
                            ui->freqCtrl->SetFrequency(static_cast<int>(frequency));                            
                            auto frequency_dbl = 0.0;
                            QString freqType;
                            if (frequency >= 1e9) {
                                frequency_dbl = frequency / 1e9;
                            } else if (frequency >= 1e6) {
                                frequency_dbl = frequency / 1e6;
                            } else if (frequency >= 1e3) {
                                frequency_dbl = frequency / 1e3;
                            } else {
                                frequency_dbl = frequency;
                            }
                            ui->m_lEditFreq->setText(QString::number(frequency_dbl, 'f', 2));                            

                        } else {
                            qDebug() << "Conversion failed for value:" << valueStr;
                            qDebug() << "Error string:" << valueStr;
                        }
                    }
                    else {
                        qDebug() << "Unknown command received: " << command;
                    }
                }
            }
            break;

        case mGetPtt:
            if (parsedValue.size() == 1)
            {
                bool pttValue = (parsedValue.at(0) != 0);                    
                hackRfManager->setPtt(pttValue);
            }
            break;        
        case mGetDeMod:
            {
                HackRfManager::Demod selectedDemod = static_cast<HackRfManager::Demod>(value);
                ui->m_cDemod->setCurrentIndex(selectedDemod);
                currentDemod = selectedDemod;
                hackRfManager->setDemod(currentDemod);
                tcpServer->setDemod(currentDemod);                
            }
            break;
        case mGetFreqMod:
        {
                HackRfManager::FreqMod selectedFreqDemod = static_cast<HackRfManager::FreqMod>(value);
                ui->m_cFreqType->setCurrentIndex(selectedFreqDemod);
                currentFreqMod = selectedFreqDemod;
        }
        break;
        default:
            qDebug() << "Unknown command. Parsed Value: " << parsedValue.toHex();
            break;
        }
    }
}

void MainWindow::initButtons(){
    /* Init Buttons */
    ui->m_pBConnect->setText("Connect");
    ui->m_pBSpeak->setText("Ptt Off");
    ui->m_pBSpeak->setEnabled(false);
}

void MainWindow::getInfo(const QString &status)
{
    ui->m_textStatus->append(status);
}

void MainWindow::on_ConnectClicked()
{
    if(ui->m_pBConnect->text() == QString("Connect"))
    {
        m_bleConnection->startScan();
    }
    else
    {       
        m_bleConnection->disconnectFromDevice();
    }
}

void MainWindow::createMessage(uint8_t msgId, uint8_t rw, QByteArray payload, QByteArray *result)
{
    uint8_t buffer[MaxPayload+8] = {'\0'};
    uint8_t command = msgId;

    int len = message.create_pack(rw , command , payload, buffer);

    for (int i = 0; i < len; i++)
    {
        result->append(static_cast<char>(buffer[i]));
    }
}

void MainWindow::parseMessage(QByteArray *data, uint8_t &command, QByteArray &value,  uint8_t &rw)
{
    MessagePack parsedMessage;

    uint8_t* dataToParse = reinterpret_cast<uint8_t*>(data->data());
    QByteArray returnValue;
    if(message.parse(dataToParse, static_cast<uint8_t>(data->length()), &parsedMessage))
    {
        command = parsedMessage.command;
        rw = parsedMessage.rw;
        for(int i = 0; i< parsedMessage.len; i++)
        {
            value.append(static_cast<char>(parsedMessage.data[i]));
        }
    }
}

void MainWindow::requestData(uint8_t command)
{
    QByteArray payload;
    QByteArray sendData;
    createMessage(command, mRead, payload, &sendData);
    m_bleConnection->writeData(sendData);
}

void MainWindow::sendCommand(uint8_t command, uint8_t value)
{
    // Convert the uint32_t value to a QByteArray
    QByteArray payload;
    payload.append(reinterpret_cast<const char*>(&value), sizeof(value));

    // Create the message and send it
    QByteArray sendData;
    createMessage(command, mWrite, payload, &sendData);

    m_bleConnection->writeData(sendData);
}

void MainWindow::sendString(uint8_t command, QByteArray value)
{
    QByteArray sendData;
    createMessage(command, mWrite, value, &sendData);
    m_bleConnection->writeData(sendData);
}

void MainWindow::on_Exit()
{
    exit(0);
}

void MainWindow::on_m_pBSpeak_clicked()
{   
    if (ui->m_pBSpeak->text() == "Ptt Off")
    {
        ui->m_pBSpeak->setText("Ptt On");
        sendCommand(mSetPtt, static_cast<uint8_t>(1));
    }
    else
    {
        ui->m_pBSpeak->setText("Ptt Off");
        sendCommand(mSetPtt, static_cast<uint8_t>(0));
    }
}

void MainWindow::on_m_pBSetFreq_clicked()
{
    if(!m_connected)
        return;

    QString freq_text = ui->m_lEditFreq->text();
    bool conversionOk;

    auto freq = freq_text.toDouble(&conversionOk);

    if (currentFreqMod == HackRfManager::FreqMod::KHZ) {
        freq = 1000 * freq;
    } else if (currentFreqMod == HackRfManager::FreqMod::MHZ) {
        freq = 1000 * 1000 * freq;
    }else if (currentFreqMod == HackRfManager::FreqMod::GHZ) {
        freq = 1000 * 1000 * 1000 * freq;
    }

    QString combinedText = QString("set_freq,%1").arg(freq);
    QByteArray data = combinedText.toUtf8();
    sendString(mData, data);
}

void MainWindow::setIp()
{
    auto ipAddress = tcpServer->getServerIpAddress();
    QString combinedText = QString("set_ip,%1").arg(ipAddress);
    QByteArray data = combinedText.toUtf8();
    sendString(mData, data);
}

void MainWindow::getBuffer(QByteArray &buffer)
{
   if(hackRfManager && m_connected)
        hackRfManager->setBuffer(buffer);
}

void MainWindow::on_m_pReset_clicked()
{
     tcpServer->reset();
     ui->m_textStatus->clear();

     ui->m_cFreqType->setCurrentIndex(2);
     ui->m_cDemod->setCurrentIndex(1);

     if(!m_connected)
        return;

     auto freq = 100;
     if (currentFreqMod == HackRfManager::FreqMod::KHZ) {
        freq = 1000 * freq;
     } else if (currentFreqMod == HackRfManager::FreqMod::MHZ) {
        freq = 1000 * 1000 * freq;
     }else if (currentFreqMod == HackRfManager::FreqMod::GHZ) {
        freq = 1000 * 1000 * 1000 * freq;
     }
     QString combinedText = QString("set_freq,%1").arg(freq);
     QByteArray data = combinedText.toUtf8();
     sendString(mData, data);
}

void MainWindow::on_m_pIncFreq_clicked()
{
     if(!m_connected)
        return;

    sendCommand(mIncFreq, static_cast<uint8_t>(1));
}

void MainWindow::on_m_pDecFreq_clicked()
{
    if(!m_connected)
        return;

    sendCommand(mDecFreq, static_cast<uint8_t>(1));
}

void MainWindow::on_m_cFreqType_currentIndexChanged(int index)
{
    if(!m_connected)
        return;

    int selectedIndex = ui->m_cFreqType->currentIndex();
    if (selectedIndex >= 0 && selectedIndex < ui->m_cFreqType->count())
    {
        HackRfManager::FreqMod freqMod = static_cast<HackRfManager::FreqMod>(selectedIndex);
        sendCommand(mSetFreqMod, static_cast<uint8_t>(freqMod));
    }
}

void MainWindow::on_m_cDemod_currentIndexChanged(int index)
{
    if(!m_connected)
        return;

    if (index >= 0 && index < ui->m_cDemod->count())
    {
        HackRfManager::Demod demod = static_cast<HackRfManager::Demod>(index);
        sendCommand(mSetDeMod, static_cast<uint8_t>(demod));
    }
}

