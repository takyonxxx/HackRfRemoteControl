#include "hackrfmanager.h"

HackRfManager::HackRfManager(QObject *parent) :
    QObject(parent), m_ptt(false), m_abort(false)
{
    // Check device
    if (rtlsdr_get_device_count() == 0)
    {
        qDebug()  << "Can not open RTL2832 USB device";
        return;
    }

    tunerFrequency  = DEFAULT_FREQUENCY;
    sampleRate      = static_cast<qint64>(DEFAULT_SAMPLE_RATE);
    fftSize         = DEFAULT_FFT_SIZE;
    fftrate         = DEFAULT_FFT_RATE;
    m_HiCutFreq     = DEFAULT_HICUT_FREQ;

    gattServer = GattServer::getInstance();
    if (gattServer)
    {
        qDebug() << "Starting gatt service";
        QObject::connect(gattServer, &GattServer::connectionState, this, &HackRfManager::onConnectionStatedChanged);
        QObject::connect(gattServer, &GattServer::dataReceived, this, &HackRfManager::onDataReceived);
        QObject::connect(gattServer, &GattServer::sendInfo, this, &HackRfManager::onInfoReceived);
        gattServer->startBleService();
    }

    currentDemod    = DemodulatorCtrl::DEMOD_WFM;
    currentFreqMod  = FreqMod::MHZ;

    m_Receiver = new Receiver();
    if (!m_Receiver) return;

    QObject::connect(m_Receiver, &Receiver::started, this, &HackRfManager::onReceiverStarted);
    QObject::connect(m_Receiver, &Receiver::stopped, this, &HackRfManager::onReceiverStopped);
    QObject::connect(m_Receiver, &Receiver::bufferProcessed, this, &HackRfManager::onBufferProcessed);

    m_Demodulator = m_Receiver->demod();
    if (!m_Demodulator) return;

    m_Demodulator->setDemod(currentDemod);
    m_Demodulator->setRrate(fftrate);
    m_Demodulator->enableAGC(true);

    m_Demodulator->setFilterWidth(2 * m_HiCutFreq, true);

    QObject::connect(m_Demodulator, &DemodulatorCtrl::spectrumUpdated, this, &HackRfManager::fftTimeout);
    QObject::connect(m_Demodulator, &DemodulatorCtrl::filterChanged, this, &HackRfManager::onFilterChanged);

    tcpClient = new TcpClient(this);
    m_Receiver->start();
}

HackRfManager::~HackRfManager()
{
    if(m_Receiver)
    {
        if(m_Receiver->isRunning())
            m_Receiver->stop();
        delete m_Receiver;
    }

    if (udpClient) {
        delete udpClient;
    }

    if (tcpClient) {
        delete tcpClient;
    }

    if (m_Demodulator) {
        delete m_Demodulator;
    }

    if (gattServer) {
        gattServer->disconnect();
        delete gattServer;
    }
}

//audio buffer
void HackRfManager::onBufferProcessed(const sdr::Buffer<int16_t> &buffer)
{
    if(!m_ptt)
    {
        if (tcpClient) {
            QByteArray soundBuffer(buffer.data(), buffer.bytesLen());
            tcpClient->sendData(soundBuffer);
        }
    }
}

void HackRfManager::onReceiverStarted()
{
    qDebug() << "ReceiverStarted";
}

void HackRfManager::onReceiverStopped()
{
    qDebug() << "ReceiverStopped";
}

void HackRfManager::fftTimeout()
{
//    auto fftsize = static_cast<unsigned int>(m_Demodulator->fftSize());
//    if (fftsize > MAX_FFT_SIZE)
//        fftsize = MAX_FFT_SIZE;

//    auto d_fftData = m_Demodulator->spectrum();
//    if (fftsize == 0)
//    {
//        return;
//    }
//    QByteArray dataBuffer(d_fftData.data(), d_fftData.bytesLen());
//    udpClient->sendFftData(dataBuffer);
}

void HackRfManager::onFilterChanged()
{
    m_HiCutFreq     = m_Demodulator->filterUpper();
    m_LowCutFreq    = m_Demodulator->filterLower();
}

void HackRfManager::createMessage(uint8_t msgId, uint8_t rw, QByteArray payload, QByteArray *result)
{
    uint8_t buffer[MaxPayload+8] = {'\0'};
    uint8_t command = msgId;

    int len = message.create_pack(rw , command , payload, buffer);

    for (int i = 0; i < len; i++)
    {
        result->append(buffer[i]);
    }
}

bool HackRfManager::parseMessage(QByteArray *data, uint8_t &command, QByteArray &value, uint8_t &rw)
{
    MessagePack parsedMessage;

    uint8_t* dataToParse = reinterpret_cast<uint8_t*>(data->data());

    if(message.parse(dataToParse, (uint8_t)data->length(), &parsedMessage))
    {
        command = parsedMessage.command;
        rw = parsedMessage.rw;

        for(int i = 0; i< parsedMessage.len; i++)
        {
            value.append(parsedMessage.data[i]);
        }

        return true;
    }
    return false;
}

void HackRfManager::onConnectionStatedChanged(bool state)
{
    if(state)
    {
        qDebug() << "Bluetooth connection is succesfull.";
    }
    else
    {
        qDebug() << "Bluetooth connection lost.";
    }
}

void HackRfManager::onDataReceived(QByteArray data)
{
    uint8_t parsedCommand;
    uint8_t rw;
    QByteArray parsedValue;
    auto parsed = parseMessage(&data, parsedCommand, parsedValue, rw);

    if(!parsed)return;

    bool ok;
    int value =  parsedValue.toHex().toInt(&ok, 16);

    if(rw == mWrite)
    {
        switch (parsedCommand)
        {
        case mSetPtt:
        {
            if(value == 1)
            {
                m_ptt = true;
                if (!m_Receiver->isRunning()) { return; }
                m_Receiver->stop();
                qDebug() << "Ptt On";
            }
            else
            {
                m_ptt = false;
                if (m_Receiver->isRunning()) { return; }
                m_Receiver->start();
                qDebug() << "Ptt Off";
            }            

            sendCommand(mGetPtt, static_cast<uint8_t>(m_ptt));
            break;
        }
        case mSetFreqMod:
        {
            if (value >= FreqMod::HZ && value <= FreqMod::GHZ)
            {
                FreqMod selectedFreqMod = static_cast<FreqMod>(value);
                currentFreqMod = selectedFreqMod;
                sendCommand(mGetFreqMod, static_cast<uint8_t>(selectedFreqMod));
                qDebug() << "Current freq mod:" << currentFreqMod;
            }
            break;
        }
        case mSetDeMod:
        {
            if (value >= DemodulatorCtrl::DEMOD_AM && value <= DemodulatorCtrl::DEMOD_BPSK31)
            {
                DemodulatorCtrl::Demod selectedDemod = static_cast<DemodulatorCtrl::Demod>(value);
                currentDemod = selectedDemod;
                m_Demodulator->setDemod(currentDemod);                
                sendCommand(mGetDeMod, static_cast<uint8_t>(selectedDemod));
                qDebug() << "Current demod:" << enumToString(static_cast<HackRfManager::Demod>(currentDemod));
            }
            break;
        }
        case mIncFreq:
        {
            auto current_freq =  m_Receiver->tunerFrequency();
            double increment = 1.0;

            switch (currentFreqMod) {
            case HZ:
                // Do nothing, as the increment is in Hertz
                break;
            case KHZ:
                increment *= 1e3; // Convert increment to Kilohertz
                break;
            case MHZ:
                increment *= 1e6; // Convert increment to Megahertz
                break;
            case GHZ:
                increment *= 1e9; // Convert increment to Gigahertz
                break;
            }
            // Update the tuner frequency
            m_Receiver->setTunerFrequency(current_freq + increment);
            current_freq =  m_Receiver->tunerFrequency();
            QString combinedText = QString("get_freq,%1").arg(current_freq);
            QByteArray data = combinedText.toUtf8();
            sendString(mData, data);
            break;
        }
        case mDecFreq:
        {
            auto current_freq =  m_Receiver->tunerFrequency();
            double increment = 1.0;

            switch (currentFreqMod) {
            case HZ:
                // Do nothing, as the increment is in Hertz
                break;
            case KHZ:
                increment *= 1e3; // Convert increment to Kilohertz
                break;
            case MHZ:
                increment *= 1e6; // Convert increment to Megahertz
                break;
            case GHZ:
                increment *= 1e9; // Convert increment to Gigahertz
                break;
            }
            // Update the tuner frequency
            m_Receiver->setTunerFrequency(current_freq - increment);
            current_freq =  m_Receiver->tunerFrequency();
            QString combinedText = QString("get_freq,%1").arg(current_freq);
            QByteArray data = combinedText.toUtf8();
            sendString(mData, data);
            break;
        }
        case mData:
        {
            auto text = QString(parsedValue.data());
            QStringList parts = text.split(',');

            // Extract the command and value
            QString command = parts.value(0);
            QString valueString = parts.value(1);

            // Convert the value string to uint64_t to handle large values
            bool conversionOkValue;
            auto value = valueString.toDouble(&conversionOkValue);            
            if (command == "set_freq") {
                m_Receiver->setTunerFrequency(value);
                auto current_freq =  m_Receiver->tunerFrequency();
                QString combinedText = QString("get_freq,%1").arg(current_freq);
                QByteArray data = combinedText.toUtf8();
                sendString(mData, data);

                if (current_freq >= 1e9) {                    
                    currentFreqMod = FreqMod::GHZ;
                    qDebug() << "Set frequency:" << current_freq / 1e9 << "GHz";
                } else if (current_freq >= 1e6) {                    
                    currentFreqMod = FreqMod::MHZ;
                    qDebug() << "Set frequency:" << current_freq / 1e6 << "MHz";
                } else if (current_freq >= 1e3) {                    
                    currentFreqMod = FreqMod::KHZ;
                    qDebug() << "Set frequency:" << current_freq / 1e3 << "KHz";
                } else {                    
                    currentFreqMod = FreqMod::HZ;
                    qDebug() << "Set frequency:" << current_freq << "Hz";
                }
                sendCommand(mGetFreqMod, static_cast<uint8_t>(currentFreqMod));
            }
            else if (command == "set_ip") {                
                tcpClient->connectToServer(valueString, 5001);
            }
            break;
        }
        default:
            break;
        }
    }
    else if(rw == mRead)
    {
        switch (parsedCommand)
        {
        case mGetFreq:
        {
            auto current_freq =  m_Receiver->tunerFrequency();
            QString combinedText = QString("get_freq,%1").arg(current_freq);
            QByteArray data = combinedText.toUtf8();
            sendString(mData, data);
            break;
        }
        case mGetFreqMod:
        {
            sendCommand(mGetFreqMod, static_cast<uint8_t>(currentFreqMod));
            break;
        }
        case mGetDeMod:
        {
            sendCommand(mGetDeMod, static_cast<uint8_t>(currentDemod));
            break;
        }
        case mGetPtt:
        {
            sendCommand(mGetPtt, static_cast<uint8_t>(m_ptt));
            break;
        }
        default:
            break;
        }
    }
}

void HackRfManager::sendCommand(uint8_t command, uint8_t value)
{
    // Convert the uint32_t value to a QByteArray
    QByteArray payload;
    payload.append(reinterpret_cast<const char*>(&value), sizeof(value));

    // Create the message and send it
    QByteArray sendData;
    createMessage(command, mWrite, payload, &sendData);
    gattServer->writeValue(sendData);
}

void HackRfManager::sendString(uint8_t command, const QString& value)
{
    QByteArray sendData;
    QByteArray bytedata;
    bytedata = value.toLocal8Bit();
    createMessage(command, mWrite, bytedata, &sendData);
    gattServer->writeValue(sendData);
}

void HackRfManager::setAbort(bool newAbort)
{
    m_abort = newAbort;
}

void HackRfManager::onInfoReceived(QString info)
{
    qDebug() << info;
}
