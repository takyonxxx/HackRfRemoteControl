#include "hackrfmanager.h"

#define HANDLE_ERROR(format, ...) this->handle_error(status, format, ##__VA_ARGS__)

HackRfManager::HackRfManager(QObject *parent) :
    QObject(parent), _device(nullptr), modulationIndex(0.0)
{
    sampleRate = DEFAULT_SAMPLE_RATE;
    currentFrequency = DEFAULT_FREQUENCY;
    m_device_mode    = STANDBY_MODE;
    m_is_initialized = false;

    audioOutputThread = new AudioOutputThread(this, 44100.);
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

    if (audioOutputThread) {
        delete audioOutputThread;
    }
}

void HackRfManager::handle_error(int status, const char * format, ...)
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
        exit(1);
    }
}

void HackRfManager::start()
{
    int status = -1;    
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

    uint32_t bandWidth = hackrf_compute_baseband_filter_bw(uint32_t(200e3));
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
    status = hackrf_set_antenna_enable(this->_device, 1);
    HANDLE_ERROR("Failed to enable antenna DC bias: %%s\n");    

    m_is_initialized = true;

    set_sample_rate(sampleRate);
    set_center_freq(currentFrequency);

    gattServer = GattServer::getInstance();
    if (gattServer)
    {
        qDebug() << "Starting gatt service";
        QObject::connect(gattServer, &GattServer::connectionState, this, &HackRfManager::onConnectionStatedChanged);
        QObject::connect(gattServer, &GattServer::dataReceived, this, &HackRfManager::onDataReceived);
        QObject::connect(gattServer, &GattServer::sendInfo, this, &HackRfManager::onInfoReceived);
        gattServer->startBleService();
    }

    StartRx();

//    QTimer* frequencyChangeTimer = new QTimer(this);
//    frequencyChangeTimer->setInterval(100); // Change frequency every 100ms
//    connect(frequencyChangeTimer, &QTimer::timeout, this, &HackRfManager::onFrequencyChangeTimer);
//    frequencyChangeTimer->start();
}

void HackRfManager::onFrequencyChangeTimer()
{
    // Implement the logic to change the frequency in steps of 10kHz
    // Here, I'm assuming you have a variable like 'currentFrequency' to keep track of the current frequency.
    // You'll need to adjust this based on your actual implementation.

    const int frequencyStep = 1e6; // Step size: 1MHz
    currentFrequency += frequencyStep;

    // Set the new frequency
    set_center_freq(currentFrequency);

    // Optionally, check if you reached a maximum frequency and stop the timer if needed
    // if (currentFrequency >= maxFrequency) {
    //     frequencyChangeTimer->stop();
    // }
}

bool HackRfManager::StartRx()
{    
    int status = hackrf_start_rx(this->_device,
                                 _hackRF_rx_callback,
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
                                 _hackRF_tx_callback,
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
            qDebug() << "Freq : " << fc_hz;
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

int HackRfManager::_hackRF_rx_callback(hackrf_transfer* transfer)
{
    HackRfManager * obj = (HackRfManager *)transfer->rx_ctx;
    return obj->hackRF_rx_callback(transfer);
}

int HackRfManager::hackRF_rx_callback(hackrf_transfer* transfer)
{
//    qDebug() << transfer->valid_length;

    int buffer_size = transfer->buffer_length / 2;
    double* demodulated_samples = new double[buffer_size];

    for (int i = 0; i < transfer->buffer_length / 2; i += 2)
    {
        // Demodulate the FM signal (frequency demodulation)
        double real = transfer->buffer[i];
        double imag = transfer->buffer[i + 1];

        // Compute the phase angle
        double phase = atan2(imag, real);

        // Calculate frequency deviation (change in phase)
        double delta_phase = phase - previous_phase;
        previous_phase = phase;

        // Perform frequency demodulation to get audio signal
        double demodulated_fm_sample = (delta_phase / (2.0 * M_PI * sampleRate)) * 1e6; // Convert to Hz
        demodulated_samples[i / 2] = demodulated_fm_sample;
    }

    if (audioOutputThread)
    {
        // Convert double array to QByteArray
        QByteArray byteArray(reinterpret_cast<const char*>(demodulated_samples), buffer_size * sizeof(double));
        audioOutputThread->writeBuffer(byteArray);
    }

    // Clean up memory
    delete[] demodulated_samples;

    return 0; // TODO: return -1 on error/stop
}

int  HackRfManager::_hackRF_tx_callback(hackrf_transfer *transfer) {
    HackRfManager *obj = (HackRfManager *)transfer->tx_ctx;
    return obj->hackRF_tx_callback(transfer);
}

int HackRfManager::hackRF_tx_callback(hackrf_transfer* transfer)
{
    qDebug() << "Hackrf tx call back called with: " << transfer->buffer_length << " bytes";
    // Example: Generate a sine wave for FM modulation
    double frequency = calculateFrequency(currentFrequency, deviation, modulationIndex);
    modulationIndex += 0.01; // Increase the modulation index for variation

    // FM modulation: Multiply the buffer by a sinusoidal waveform
    for (size_t i = 0; i < transfer->buffer_length; ++i) {
        double modulation = std::sin(2 * M_PI * frequency * i / sampleRate);
        transfer->buffer[i] = static_cast<int16_t>(transfer->buffer[i] * modulation);
    }

    // Transmit the sample
    hackrf_set_freq(this->_device, frequency);
    QThread::msleep(10);
    return 0; // TODO: return -1 on error/stop
}

void HackRfManager::onInfoReceived(QString info)
{
    qDebug() << info;
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

