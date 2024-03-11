#include "sdrdevice.h"

SdrDevice::SdrDevice(QObject *parent):
    QThread(parent)
{
    m_ptt                  = false;
    currentDemod    = Demod::DEMOD_WFM;
    currentFreqMod  = FreqMod::MHZ;

    sample_rate            = DEFAULT_SAMPLE_RATE;
    audio_samp_rate        = DEFAULT_AUDIO_SAMPLE_RATE;
    currentFrequency       = DEFAULT_FREQUENCY;
    cut_off                = DEFAULT_CHANNEL_WIDTH;
    transition             = static_cast<int>(DEFAULT_CHANNEL_WIDTH / 12);
    audio_gain             = DEFAULT_AUDIO_GAIN;

    decimation             = static_cast<int>(DEFAULT_SAMPLE_RATE/DEFAULT_CHANNEL_WIDTH);
    interpolation          = static_cast<int>(DEFAULT_SAMPLE_RATE / 1e6);
    resampler_decimation   = static_cast<int>(DEFAULT_SAMPLE_RATE * decimation / 1e6);

    try {
//        hackrf_osmo_source = osmosdr::source::make("hackrf=0");
//        hackrf_osmo_source->set_time_unknown_pps(osmosdr::time_spec_t());
//        hackrf_osmo_source->set_sample_rate(sample_rate);
//        hackrf_osmo_source->set_center_freq(currentFrequency, 0);
//        hackrf_osmo_source->set_freq_corr(0, 0);
//        hackrf_osmo_source->set_dc_offset_mode(0, 0);
//        hackrf_osmo_source->set_iq_balance_mode(0, 0);
//        hackrf_osmo_source->set_gain_mode(false, 0);
//        hackrf_osmo_source->set_gain(0, 0);
//        hackrf_osmo_source->set_if_gain(40, 0);
//        hackrf_osmo_source->set_bb_gain(40, 0);
//        hackrf_osmo_source->set_antenna("", 0);
//        hackrf_osmo_source->set_bandwidth(0, 0);

//        qDebug() << "Channel Count: " + QString::number(hackrf_osmo_source->get_num_channels());
//        qDebug() << "Center Frequency: " << hackrf_osmo_source->get_center_freq(0) << " Hz";
//        qDebug() << "Sample Rate: " << hackrf_osmo_source->get_sample_rate() << " Hz";
//        qDebug() << "Actual RX Gain: " << hackrf_osmo_source->get_gain() << " dB...";
//        qDebug() << "IF Gain: " << hackrf_osmo_source->get_gain("IF", 0) << " dB";
//        qDebug() << "BB Gain: " << hackrf_osmo_source->get_gain("BB", 0) << " dB";

        std::string dev = "hackrf=0";
        std::string stream_args = "";
        std::vector<std::string> tune_args = {""};
        std::vector<std::string> settings = {""};

        hackrf_soapy_source = gr::soapy::source::make(
            "hackrf",
            "fc32",
            1,
            dev,
            stream_args,
            tune_args,
            settings
            );

        if (!hackrf_soapy_source) {
            throw std::runtime_error("Failed to create SoapySDR source.");
        }

        hackrf_soapy_source->set_sample_rate(0, sample_rate);
        hackrf_soapy_source->set_bandwidth(0, 0);
        hackrf_soapy_source->set_frequency(0, currentFrequency);
        hackrf_soapy_source->set_gain(0, "AMP", false);
        hackrf_soapy_source->set_gain(0, "LNA", std::min(std::max(40.0, 0.0), 40.0));
        hackrf_soapy_source->set_gain(0, "VGA", std::min(std::max(40.0, 0.0), 62.0));

        // Print device information
        qDebug() << "Center Frequency: " << hackrf_soapy_source->get_frequency(0) << " Hz";
        qDebug() << "Sample Rate: " << hackrf_soapy_source->get_sample_rate(0) << " Hz";
        qDebug() << "Actual RX Gain: " << hackrf_soapy_source->get_gain(0) << " dB...";
        qDebug() << "LNA Gain: " << hackrf_soapy_source->get_gain(0, "LNA") << " dB";
        qDebug() << "VGA Gain: " << hackrf_soapy_source->get_gain(0, "VGA") << " dB";

    } catch (const std::exception &e) {
        qDebug() << "Source Error: " << e.what();
    }

    customBuffer = std::make_shared<CustomBuffer>("custom_buffer");
    gattServer = GattServer::getInstance();
    if (gattServer)
    {
        qDebug() << "Starting gatt service";
        QObject::connect(gattServer, &GattServer::connectionState, this, &SdrDevice::onConnectionStatedChanged);
        QObject::connect(gattServer, &GattServer::dataReceived, this, &SdrDevice::onDataReceived);
        QObject::connect(gattServer, &GattServer::sendInfo, this, &SdrDevice::onInfoReceived);
        gattServer->startBleService();
    }
}

SdrDevice::~SdrDevice()
{       
}

void SdrDevice::setFrequency(double frequency)
{
//    if (hackrf_osmo_source) {
//        hackrf_osmo_source->set_center_freq(frequency);
//        currentFrequency = getCenterFrequency();
//    }
    hackrf_soapy_source->set_frequency(0, frequency);
    currentFrequency = getCenterFrequency();
}

double SdrDevice::getCenterFrequency() const
{
//    if (hackrf_osmo_source)
//    {
//        return hackrf_osmo_source->get_center_freq();
//    }
    return hackrf_soapy_source->get_frequency(0);
}

void SdrDevice::onConnectionStatedChanged(bool state)
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

void SdrDevice::onDataReceived(QByteArray data)
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
                qDebug() << "Set Ptt On";
            }
            else
            {
                m_ptt = false;
                qDebug() << "Set Ptt Off";
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
                qDebug() << "Set freq mod:" << enumFreqModToString(static_cast<FreqMod>(currentFreqMod));;
            }
            break;
        }
        case mSetDeMod:
        {
            if (value >= Demod::DEMOD_AM && value <=Demod::DEMOD_WFM)
            {
                Demod selectedDemod = static_cast<Demod>(value);
                currentDemod = selectedDemod;
                sendCommand(mGetDeMod, static_cast<uint8_t>(selectedDemod));
                qDebug() << "Set demod:" << enumDemodToString(static_cast<Demod>(currentDemod));
            }
            break;
        }
        case mIncFreq:
        {            
            auto increment = value;

            switch (currentFreqMod) {
            case FreqMod::HZ:
                // Do nothing, as the increment is in Hertz
                break;
            case FreqMod::KHZ:
                increment *= 1e3; // Convert increment to Kilohertz
                break;
            case FreqMod::MHZ:
                increment *= 1e6; // Convert increment to Megahertz
                break;
            case FreqMod::GHZ:
                increment *= 1e9; // Convert increment to Gigahertz
                break;
            }

            // Update the tuner frequency
            setFrequency(currentFrequency + increment);
            currentFrequency =  getCenterFrequency();
            qDebug() << "Set frequency:" << currentFrequency << "Hz";
            QString combinedText = QString("get_freq,%1").arg(currentFrequency);
            QByteArray data = combinedText.toUtf8();
            sendString(mData, data);
            break;
        }
        case mDecFreq:
        {
            auto increment = value;

            switch (currentFreqMod) {
            case FreqMod::HZ:
                // Do nothing, as the increment is in Hertz
                break;
            case FreqMod::KHZ:
                increment *= 1e3; // Convert increment to Kilohertz
                break;
            case FreqMod::MHZ:
                increment *= 1e6; // Convert increment to Megahertz
                break;
            case FreqMod::GHZ:
                increment *= 1e9; // Convert increment to Gigahertz
                break;
            }            

            // Update the tuner frequency
            setFrequency(currentFrequency - increment);
            currentFrequency =  getCenterFrequency();
            qDebug() << "Set frequency:" << currentFrequency << "Hz";
            QString combinedText = QString("get_freq,%1").arg(currentFrequency);
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

                setFrequency(value);

                currentFrequency = getCenterFrequency();

                if (currentFreqMod == FreqMod::GHZ) {
                    currentFreqMod = FreqMod::GHZ;
                    qDebug() << "Set frequency:" << value / 1e9 << "GHz";
                } else if (currentFreqMod == FreqMod::MHZ) {
                    currentFreqMod = FreqMod::MHZ;
                    qDebug() << "Set frequency:" << value / 1e6 << "MHz";
                } else if (currentFreqMod == FreqMod::KHZ) {
                    currentFreqMod = FreqMod::KHZ;
                    qDebug() << "Set frequency:" << value / 1e3 << "KHz";
                } else {
                    currentFreqMod = FreqMod::HZ;
                    qDebug() << "Set frequency:" << value << "Hz";
                }

                sendCommand(mGetFreqMod, static_cast<uint8_t>(currentFreqMod));

                QString combinedText = QString("get_freq,%1").arg(currentFrequency);
                QByteArray data = combinedText.toUtf8();
                sendString(mData, data);
            }
            else if (command == "set_ip") {
                customBuffer->connectToServer(valueString, 5001);
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
            qDebug() << "Current Freq:" << currentFrequency;
            sendString(mData, data);
            break;
        }
        case mGetFreqMod:
        {
            qDebug() << "Current FreqMod:" << currentFreqMod;
            sendCommand(mGetFreqMod, static_cast<uint8_t>(currentFreqMod));
            break;
        }
        case mGetDeMod:
        {
            qDebug() << "Current Demod:" << currentDemod;
            sendCommand(mGetDeMod, static_cast<uint8_t>(currentDemod));
            break;
        }
        case mGetPtt:
        {
            qDebug() << "Current Ptt:" << m_ptt;
            sendCommand(mGetPtt, static_cast<uint8_t>(m_ptt));
            break;
        }
        default:
            break;
        }
    }
}

void SdrDevice::createMessage(uint8_t msgId, uint8_t rw, QByteArray payload, QByteArray *result)
{
    uint8_t buffer[MaxPayload+8] = {'\0'};
    uint8_t command = msgId;

    int len = message.create_pack(rw , command , payload, buffer);

    for (int i = 0; i < len; i++)
    {
        result->append(buffer[i]);
    }
}

bool SdrDevice::parseMessage(QByteArray *data, uint8_t &command, QByteArray &value, uint8_t &rw)
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

void SdrDevice::sendCommand(uint8_t command, uint8_t value)
{
    // Convert the uint32_t value to a QByteArray
    QByteArray payload;
    payload.append(reinterpret_cast<const char*>(&value), sizeof(value));

    // Create the message and send it
    QByteArray sendData;
    createMessage(command, mWrite, payload, &sendData);
    gattServer->writeValue(sendData);
}

void SdrDevice::sendString(uint8_t command, const QString& value)
{
    QByteArray sendData;
    QByteArray bytedata;
    bytedata = value.toLocal8Bit();
    createMessage(command, mWrite, bytedata, &sendData);
    gattServer->writeValue(sendData);
}

void SdrDevice::onInfoReceived(QString info)
{
    qDebug() << info;
}

void SdrDevice::run()
{
    tb = gr::make_top_block("HackRf");

    gr::filter::rational_resampler_ccf::sptr resampler = gr::filter::rational_resampler_ccf::make(interpolation, resampler_decimation);
    auto low_pass_filter = gr::filter::fir_filter_fff::make(
        6,
        gr::filter::firdes::low_pass(1, sample_rate, cut_off, transition, gr::fft::window::WIN_HAMMING, 6.76));    
    gr::analog::quadrature_demod_cf::sptr quad_demod = gr::analog::quadrature_demod_cf::make(1.0);

//  gr::blocks::null_sink::sptr null_sink = gr::blocks::null_sink::make(sizeof(gr_complex));
    auto audio_sink = gr::audio::sink::make(audio_samp_rate, "", true);
    try{       
        //  tb->connect(hackrf_osmo_source, 0, resampler, 0);
        tb->connect(hackrf_soapy_source, 0, resampler, 0);
        tb->connect(resampler, 0, quad_demod, 0);
        tb->connect(quad_demod, 0, low_pass_filter, 0);
        tb->connect(low_pass_filter, 0, audio_sink, 0);
        tb->start();
    } catch (const std::exception &e) {
        qDebug() << "Block Error: " << e.what();
    }
}
