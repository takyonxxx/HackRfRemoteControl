#include "osmodevice.h"

OsmoDevice::OsmoDevice(QObject *parent):
    QThread(parent)
{
    sample_rate            = DEFAULT_SAMPLE_RATE;
    audio_samp_rate        = DEFAULT_AUDIO_SAMPLE_RATE;
    currentFrequency       = DEFAULT_FREQUENCY;
    cut_off                = DEFAULT_CHANNEL_WIDTH;
    transition             = static_cast<int>(DEFAULT_CHANNEL_WIDTH / 12);
    decimation             = static_cast<int>(DEFAULT_SAMPLE_RATE/DEFAULT_CHANNEL_WIDTH);
    interpolation          = static_cast<int>(DEFAULT_SAMPLE_RATE / 1e6);
    resampler_decimation   = static_cast<int>(DEFAULT_SAMPLE_RATE * decimation / 1e6);
    audio_gain             = DEFAULT_AUDIO_GAIN;
    m_ptt                  = false;
    currentDemod    = OsmoDevice::Demod::DEMOD_WFM;
    currentFreqMod  = OsmoDevice::FreqMod::MHZ;

    hackrf_osmo_source = osmosdr::source::make("hackrf=0");

    hackrf_osmo_source->set_time_unknown_pps(osmosdr::time_spec_t());
    hackrf_osmo_source->set_sample_rate(sample_rate);
    hackrf_osmo_source->set_center_freq(currentFrequency, 0);
    hackrf_osmo_source->set_freq_corr(0, 0);
    hackrf_osmo_source->set_dc_offset_mode(0, 0);
    hackrf_osmo_source->set_iq_balance_mode(0, 0);
    hackrf_osmo_source->set_gain_mode(false, 0);
    hackrf_osmo_source->set_gain(0, 0);
    hackrf_osmo_source->set_if_gain(40, 0);
    hackrf_osmo_source->set_bb_gain(40, 0);
    hackrf_osmo_source->set_antenna("", 0);
    hackrf_osmo_source->set_bandwidth(0, 0);

    std::string ver = gr::version();
    qDebug() << "GNU Radio Version: " + ver;
    qDebug() << "Channel Count: " + QString::number(hackrf_osmo_source->get_num_channels());
    qDebug() << "Center Frequency: " << hackrf_osmo_source->get_center_freq(0) << " Hz";
    qDebug() << "Sample Rate: " << hackrf_osmo_source->get_sample_rate() << " Hz\n";
    qDebug() << "Actual RX Gain: " << hackrf_osmo_source->get_gain() << " dB...";
    qDebug() << "IF Gain: " << hackrf_osmo_source->get_gain("IF", 0) << " dB";
    qDebug() << "BB Gain: " << hackrf_osmo_source->get_gain("BB", 0) << " dB";

    customAudioSink = std::make_shared<CustomAudioSink>(audio_samp_rate, "audio_sink", true);

    gattServer = GattServer::getInstance();
    if (gattServer)
    {
        qDebug() << "Starting gatt service";
        QObject::connect(gattServer, &GattServer::connectionState, this, &OsmoDevice::onConnectionStatedChanged);
        QObject::connect(gattServer, &GattServer::dataReceived, this, &OsmoDevice::onDataReceived);
        QObject::connect(gattServer, &GattServer::sendInfo, this, &OsmoDevice::onInfoReceived);
        gattServer->startBleService();
    }
}

OsmoDevice::~OsmoDevice()
{       
}

void OsmoDevice::setFrequency(double frequency)
{
    if (hackrf_osmo_source) {
        hackrf_osmo_source->set_center_freq(frequency);
        currentFrequency = getCenterFrequency();
    }
}

double OsmoDevice::getCenterFrequency() const
{
    if (hackrf_osmo_source)
    {
        return hackrf_osmo_source->get_center_freq();
    }
    return currentFrequency;
}

void OsmoDevice::setSampleRate(double sampleRate)
{
    sample_rate = sampleRate;
    if (hackrf_osmo_source) {
        hackrf_osmo_source->set_sample_rate(sampleRate);
        sample_rate = getSampleRate();
    }
}

double OsmoDevice::getSampleRate()
{
    if (hackrf_osmo_source) {
        return hackrf_osmo_source->get_sample_rate();
    }
    return sample_rate;
}

void OsmoDevice::setGain(double gain)
{
    if (hackrf_osmo_source)
    {
        hackrf_osmo_source->set_if_gain(gain, 0);
        hackrf_osmo_source->set_bb_gain(gain, 0);
    }
}

void OsmoDevice::onConnectionStatedChanged(bool state)
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

void OsmoDevice::onDataReceived(QByteArray data)
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
            if (value >= OsmoDevice::FreqMod::HZ && value <= OsmoDevice::FreqMod::GHZ)
            {
                OsmoDevice::FreqMod selectedFreqMod = static_cast<OsmoDevice::FreqMod>(value);
                currentFreqMod = selectedFreqMod;
                sendCommand(mGetFreqMod, static_cast<uint8_t>(selectedFreqMod));
                qDebug() << "Set freq mod:" << currentFreqMod;
            }
            break;
        }
        case mSetDeMod:
        {
            if (value >= OsmoDevice::Demod::DEMOD_AM && value <= OsmoDevice::Demod::DEMOD_WFM)
            {
                OsmoDevice::Demod selectedDemod = static_cast<OsmoDevice::Demod>(value);
                currentDemod = selectedDemod;
                sendCommand(mGetDeMod, static_cast<uint8_t>(selectedDemod));
                qDebug() << "Set demod:" << enumToString(static_cast<OsmoDevice::Demod>(currentDemod));
            }
            break;
        }
        case mIncFreq:
        {            
            auto increment = value;

            switch (currentFreqMod) {
            case OsmoDevice::FreqMod::HZ:
                // Do nothing, as the increment is in Hertz
                break;
            case OsmoDevice::FreqMod::KHZ:
                increment *= 1e3; // Convert increment to Kilohertz
                break;
            case OsmoDevice::FreqMod::MHZ:
                increment *= 1e6; // Convert increment to Megahertz
                break;
            case OsmoDevice::FreqMod::GHZ:
                increment *= 1e9; // Convert increment to Gigahertz
                break;
            }
            qDebug() << "Inc freq:" << increment;
            // Update the tuner frequency
            setFrequency(currentFrequency + increment);
            currentFrequency =  getCenterFrequency();
            QString combinedText = QString("get_freq,%1").arg(currentFrequency);
            QByteArray data = combinedText.toUtf8();
            sendString(mData, data);
            break;
        }
        case mDecFreq:
        {
            auto increment = value;

            switch (currentFreqMod) {
            case OsmoDevice::FreqMod::HZ:
                // Do nothing, as the increment is in Hertz
                break;
            case OsmoDevice::FreqMod::KHZ:
                increment *= 1e3; // Convert increment to Kilohertz
                break;
            case OsmoDevice::FreqMod::MHZ:
                increment *= 1e6; // Convert increment to Megahertz
                break;
            case OsmoDevice::FreqMod::GHZ:
                increment *= 1e9; // Convert increment to Gigahertz
                break;
            }            
            qDebug() << "Inc freq:" << increment;
            // Update the tuner frequency
            setFrequency(currentFrequency - increment);
            currentFrequency =  getCenterFrequency();
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

                if (currentFreqMod == OsmoDevice::FreqMod::GHZ) {
                    currentFreqMod = FreqMod::GHZ;
                    qDebug() << "Set frequency:" << value / 1e9 << "GHz";
                } else if (currentFreqMod == OsmoDevice::FreqMod::MHZ) {
                    currentFreqMod = FreqMod::MHZ;
                    qDebug() << "Set frequency:" << value / 1e6 << "MHz";
                } else if (currentFreqMod == OsmoDevice::FreqMod::KHZ) {
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
//                customAudioSink->connectToServer(valueString, 5001);
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

void OsmoDevice::createMessage(uint8_t msgId, uint8_t rw, QByteArray payload, QByteArray *result)
{
    uint8_t buffer[MaxPayload+8] = {'\0'};
    uint8_t command = msgId;

    int len = message.create_pack(rw , command , payload, buffer);

    for (int i = 0; i < len; i++)
    {
        result->append(buffer[i]);
    }
}

bool OsmoDevice::parseMessage(QByteArray *data, uint8_t &command, QByteArray &value, uint8_t &rw)
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

void OsmoDevice::sendCommand(uint8_t command, uint8_t value)
{
    // Convert the uint32_t value to a QByteArray
    QByteArray payload;
    payload.append(reinterpret_cast<const char*>(&value), sizeof(value));

    // Create the message and send it
    QByteArray sendData;
    createMessage(command, mWrite, payload, &sendData);
    gattServer->writeValue(sendData);
}

void OsmoDevice::sendString(uint8_t command, const QString& value)
{
    QByteArray sendData;
    QByteArray bytedata;
    bytedata = value.toLocal8Bit();
    createMessage(command, mWrite, bytedata, &sendData);
    gattServer->writeValue(sendData);
}

void OsmoDevice::onInfoReceived(QString info)
{
    qDebug() << info;
}

void OsmoDevice::run()
{
    tb = gr::make_top_block("HackRf");

    gr::filter::rational_resampler_ccf::sptr resampler = gr::filter::rational_resampler_ccf::make(interpolation, resampler_decimation);
    gr::analog::quadrature_demod_cf::sptr quad_demod = gr::analog::quadrature_demod_cf::make(1.0);
    auto low_pass_filter = gr::filter::fir_filter_fff::make(
        6,
        gr::filter::firdes::low_pass(1, sample_rate, cut_off, transition, gr::fft::window::WIN_HAMMING, 6.76));
    // auto audio_sink = gr::audio::sink::make(audio_samp_rate, "", true);
    auto multiply_const = gr::blocks::multiply_const_ff::make(audio_gain);

    tb->connect(hackrf_osmo_source, 0, resampler, 0);
    tb->connect(resampler, 0, quad_demod, 0);
    tb->connect(quad_demod, 0, low_pass_filter, 0);
    tb->connect(low_pass_filter, 0, multiply_const, 0);
    tb->connect(multiply_const, 0, customAudioSink, 0);
    // tb->connect(multiply_const, 0, audio_sink, 0);
    tb->start();
}
