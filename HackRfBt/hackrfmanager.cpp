#include "hackrfmanager.h"

#define HANDLE_ERROR(format, ...) this->handle_error(status, format, ##__VA_ARGS__)

int _hackrf_rx_callback(hackrf_transfer *transfer) {
    HackRfManager *obj = (HackRfManager *)transfer->rx_ctx;
    return obj->HackRFRxCallback((int8_t *)transfer->buffer, transfer->valid_length);
}

int _hackrf_tx_callback(hackrf_transfer *transfer) {
    HackRfManager *obj = (HackRfManager *)transfer->tx_ctx;
    return obj->HackRFTxCallback((int8_t *)transfer->buffer, transfer->valid_length);
}

HackRfManager::HackRfManager(QObject *parent) :
    QObject(parent), _device(nullptr), modulationIndex(0.0), m_ptt(false)
{
    sampleRate = DEFAULT_SAMPLE_RATE;
    currentFrequency = DEFAULT_FREQUENCY;
    m_device_mode    = STANDBY_MODE;
    m_is_initialized = false;

    currentDemod    = HackRfManager::DEMOD_WFM;
    currentFreqMod  = FreqMod::MHZ;

    gattServer = GattServer::getInstance();
    if (gattServer)
    {
        qDebug() << "Starting gatt service";
        QObject::connect(gattServer, &GattServer::connectionState, this, &HackRfManager::onConnectionStatedChanged);
        QObject::connect(gattServer, &GattServer::dataReceived, this, &HackRfManager::onDataReceived);
        QObject::connect(gattServer, &GattServer::sendInfo, this, &HackRfManager::onInfoReceived);
        gattServer->startBleService();
    }

    tcpClient = new TcpClient(this);
    audioOutput = new AudioOutput(this, 48000);
}

HackRfManager::~HackRfManager()
{    
    if (this->_device != nullptr) {
        int status = hackrf_stop_rx(this->_device);
        status = hackrf_stop_tx(this->_device);

        status = hackrf_close(this->_device);
        HANDLE_ERROR("Error closing hackrf: %%s\n");
        _device = nullptr;
    }
    
    if (audioOutput) {
        delete audioOutput;
    }
}

int HackRfManager::HackRFRxCallback(int8_t* buffer, uint32_t length)
{
    return mRxHandler->onData(buffer, length);
}

int HackRfManager::HackRFTxCallback(int8_t* buffer, uint32_t length)
{
    return mTxHandler->onData(buffer, length);
}

bool HackRfManager::handle_error(int status, const char * format, ...)
{
    if (status != 0) {
        char buffer[256];
        va_list args;
        va_start(args, format);
        int len = vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        fprintf(stderr, "%.*s", len, buffer);
        fprintf(stderr, "%s", hackrf_error_name(static_cast<hackrf_error>(status)));
        hackrf_close(this->_device);
        hackrf_exit();
        return false;
    }
    return true;
}

bool HackRfManager::Open(IHackRFData *handler)
{
    int status = -1;

    mRxHandler = handler;

    status = hackrf_init();
    HANDLE_ERROR("hackrf_init() failed: %s\n");

    hackrf_device_list_t* devices = hackrf_device_list();
    if (devices == nullptr)
    {
        qDebug() << "hackrf_device_list failed";
    }
    else
    {
        qDebug() << "Searching hackrf devices.";
        for (int i = 0; i < devices->devicecount; i++)
        {
            qDebug() << "HackRf device S/N " << devices->serial_numbers[i] << "found";
            break;
        }
        hackrf_device_list_free(devices);
    }

    status = hackrf_open(&_device);
    HANDLE_ERROR("Failed to open HackRF device: %s\n");

    uint8_t board_id;
    status = hackrf_board_id_read( this->_device, &board_id );
    HANDLE_ERROR("Failed to get HackRF board id: %%s\n");
    qDebug() << "board_id" << board_id;

    char version[128];
    memset(version, 0, sizeof(version));
    status = hackrf_version_string_read( this->_device, version, sizeof(version));
    HANDLE_ERROR("Failed to read version string: %%s\n");
    qDebug() << "version" << version;

    uint32_t bandWidth = hackrf_compute_baseband_filter_bw(uint32_t(300e3));
    status = hackrf_set_baseband_filter_bandwidth( this->_device, bandWidth );
    HANDLE_ERROR("hackrf_set_baseband_filter_bandwidth %u: %%s", bandWidth );
    qDebug() << "bandWidth" << bandWidth;

    status = hackrf_set_hw_sync_mode(this->_device, 0);
    HANDLE_ERROR("hackrf_set_hw_sync_mode() failed: %s (%d)\n");

    /* range 0-40 step 8d, IF gain in osmosdr  */
    hackrf_set_lna_gain(this->_device, 40);

    /* range 0-62 step 2db, BB gain in osmosdr */
    hackrf_set_vga_gain(this->_device, 40);

    /* Disable AMP gain stage by default. */
    hackrf_set_amp_enable(this->_device, 0);

    /* antenna port power control */
    status = hackrf_set_antenna_enable(this->_device, 0);
    HANDLE_ERROR("Failed to enable antenna DC bias: %%s\n");

    // Additional settings for FM demodulation
    hackrf_set_txvga_gain(_device, 20);      // Set TX VGA gain

    m_is_initialized = true;

    set_sample_rate(sampleRate);
    set_center_freq(currentFrequency);

    return true;
}

bool HackRfManager::StartRx()
{
    int status = hackrf_start_rx(this->_device,
                                 _hackrf_rx_callback,
                                 (void *)this);
    HANDLE_ERROR("Failed to start RX streaming: %%s\n");
    return true;
}

bool HackRfManager::stop_Rx()
{
    if ( m_is_initialized && m_device_mode == RX_MODE )
    {
        int status = hackrf_stop_rx( this->_device );
        HANDLE_ERROR("Failed to stop RX streaming: %%s\n");
        m_device_mode = STANDBY_MODE;
        return true;
    }
    return false;
}

bool HackRfManager::StartTx()
{    
    int status = hackrf_start_tx(this->_device,
                                 _hackrf_tx_callback,
                                 (void *)this);
    HANDLE_ERROR("Failed to start TX streaming: %%s\n");
    return true;
}

bool HackRfManager::stop_Tx()
{    
    if ( m_is_initialized && m_device_mode == RX_MODE )
    {

        int status = hackrf_stop_tx( this->_device );
        HANDLE_ERROR("Failed to stop TX streaming: %%s\n");
        m_device_mode = STANDBY_MODE;
        return true;
    }
    return false;
}

bool HackRfManager::set_center_freq( double fc_hz )
{
    if ( m_is_initialized )
    {
        if ( fc_hz >= MHZ(20) && fc_hz <= GHZ(6) )
        {
            int status = hackrf_set_freq( this->_device, fc_hz );
            HANDLE_ERROR("Failed to set center frequency : %%s\n");
            currentFrequency = fc_hz;
            qDebug() << "Current Frequency : " << currentFrequency;
            return true;
        }
    }
    return false;
}

bool HackRfManager::set_sample_rate( double rate )
{
    if ( m_is_initialized )
    {
        assert(this->_device != nullptr);

        int status = HACKRF_SUCCESS;
        double _sample_rates[] = {
                                  8e6,
                                  10e6,
                                  12.5e6,
                                  16e6,
                                  20e6};

        bool found_supported_rate = false;
        for( unsigned int i = 0; i < sizeof(_sample_rates)/sizeof(double); i++ ) {
            if(_sample_rates[i] == rate) {
                found_supported_rate = true;
                break;
            }
        }

        if (!found_supported_rate) {
            status = HACKRF_ERROR_OTHER;
            HANDLE_ERROR("Unsupported samplerate: %gMsps", rate/1e6);
        }

        status = hackrf_set_sample_rate( this->_device, rate);
        HANDLE_ERROR("Error setting sample rate to %gMsps: %%s\n", rate/1e6);
        return true;
    }
    return false;
}

//int HackRfManager::hackRF_rx_callback(hackrf_transfer* transfer)
//{
//    auto gain = 40;
//    output = new float[transfer->valid_length * sampleRate * 2];

//    deviation = 2.0 * M_PI * 75.0e3 / hackrf_sample;
//    for (int i = 0; i < transfer->valid_length ; i++) {

//        double	audio_amp = transfer->buffer[i] * gain;

//        if (fabs(audio_amp) > 1.0) {
//            audio_amp = (audio_amp > 0.0) ? 1.0 : -1.0;
//        }
//        fm_phase += fm_deviation * audio_amp;
//        while (fm_phase > (float)(M_PI))
//            fm_phase -= (float)(2.0 * M_PI);
//        while (fm_phase < (float)(-M_PI))
//            fm_phase += (float)(2.0 * M_PI);

//        output[i * sampleRate] = (float)sin(fm_phase);
//        output[i * sampleRate + 1] = (float)cos(fm_phase);
//    }

//    if (!m_ptt)
//    {
//        if (tcpClient)
//        {
//            QByteArray soundBuffer(reinterpret_cast<const char*>(output), transfer->valid_length * sampleRate * 2 * sizeof(float));
//            tcpClient->sendData(soundBuffer);
//            if (audioOutputThread)
//            {
//                audioOutputThread->writeBuffer(soundBuffer);
//            }
//        }
//    }
//    delete[] output;
//    return 0;
//}

//int HackRfManager::hackRF_tx_callback(hackrf_transfer* transfer)
//{
//    qDebug() << "Hackrf tx call back called with: " << transfer->buffer_length << " bytes";
//    // Example: Generate a sine wave for FM modulation
//    double frequency = calculateFrequency(currentFrequency, deviation, modulationIndex);
//    modulationIndex += 0.01; // Increase the modulation index for variation

//    // FM modulation: Multiply the buffer by a sinusoidal waveform
//    for (size_t i = 0; i < transfer->buffer_length; ++i) {
//        double modulation = std::sin(2 * M_PI * frequency * i / sampleRate);
//        transfer->buffer[i] = static_cast<int16_t>(transfer->buffer[i] * modulation);
//    }

//    // Transmit the sample
//    hackrf_set_freq(this->_device, frequency);
//    QThread::msleep(10);
//    return 0; // TODO: return -1 on error/stop
//}

void HackRfManager::onInfoReceived(QString info)
{
    qDebug() << info;
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
                stop_Rx();
                int status = hackrf_close(this->_device);
                HANDLE_ERROR("Error closing hackrf: %%s\n");

                status = hackrf_exit();
                HANDLE_ERROR("Error exiting hackrf: %%s\n");

                _device = nullptr;
                qDebug() << "Ptt On";
            }
            else
            {
                m_ptt = false;

                Open(mRxHandler);
                StartRx();
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
            if (value >= HackRfManager::DEMOD_AM && value <= HackRfManager::DEMOD_BPSK31)
            {
                HackRfManager::Demod selectedDemod = static_cast<HackRfManager::Demod>(value);
                currentDemod = selectedDemod;
                sendCommand(mGetDeMod, static_cast<uint8_t>(selectedDemod));
                qDebug() << "Current demod:" << enumToString(static_cast<HackRfManager::Demod>(currentDemod));
            }
            break;
        }
        case mIncFreq:
        {
            auto current_freq =  currentFrequency;
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
            set_center_freq(current_freq + increment);
            currentFrequency =  current_freq + increment;
            QString combinedText = QString("get_freq,%1").arg(current_freq);
            QByteArray data = combinedText.toUtf8();
            sendString(mData, data);
            break;
        }
        case mDecFreq:
        {
            auto current_freq = currentFrequency;
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
            set_center_freq(current_freq - increment);
            currentFrequency =  current_freq - increment;
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
                set_center_freq(value);
                currentFrequency = value;
                auto current_freq =  currentFrequency;
                QString combinedText = QString("get_freq,%1").arg(current_freq);
                QByteArray data = combinedText.toUtf8();
                sendString(mData, data);

                if (current_freq >= 1e9) {
                    currentFreqMod = FreqMod::GHZ;
                    qDebug() << "Current frequency:" << current_freq / 1e9 << "GHz";
                } else if (current_freq >= 1e6) {
                    currentFreqMod = FreqMod::MHZ;
                    qDebug() << "Current frequency:" << current_freq / 1e6 << "MHz";
                } else if (current_freq >= 1e3) {
                    currentFreqMod = FreqMod::KHZ;
                    qDebug() << "Current frequency:" << current_freq / 1e3 << "KHz";
                } else {
                    currentFreqMod = FreqMod::HZ;
                    qDebug() << "Current frequency:" << current_freq << "Hz";
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
            auto current_freq =  currentFrequency;
            QString combinedText = QString("get_freq,%1").arg(current_freq);
            QByteArray data = combinedText.toUtf8();
            sendString(mData, data);
            qDebug() << "Get freq:" << currentFrequency;
            break;
        }
        case mGetFreqMod:        {
            sendCommand(mGetFreqMod, static_cast<uint8_t>(currentFreqMod));
            qDebug() << "Get freq mod:" << currentFreqMod;
            break;
        }
        case mGetDeMod:        {
            sendCommand(mGetDeMod, static_cast<uint8_t>(currentDemod));
            qDebug() << "Get demod:" << currentDemod;
            break;
        }
        case mGetPtt:        {
            sendCommand(mGetPtt, static_cast<uint8_t>(m_ptt));
            qDebug() << "Get ptt:" << m_ptt;
            break;
        }
        default:
            break;
        }
    }
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

