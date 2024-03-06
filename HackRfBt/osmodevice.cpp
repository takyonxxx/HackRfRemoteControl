#include "osmodevice.h"

OsmoDevice::OsmoDevice(QObject *parent):
    QThread(parent)
{
    sample_rate            = DEFAULT_SAMPLE_RATE;
    audio_samp_rate        = DEFAULT_AUDIO_SAMPLE_RATE;
    center_freq            = DEFAULT_FREQUENCY;
    cut_off                = DEFAULT_CHANNEL_WIDTH;
    transition             = static_cast<int>(DEFAULT_CHANNEL_WIDTH / 12);
    decimation             = static_cast<int>(DEFAULT_SAMPLE_RATE/DEFAULT_CHANNEL_WIDTH);
    interpolation          = static_cast<int>(DEFAULT_SAMPLE_RATE / 1e6);
    resampler_decimation   = static_cast<int>(DEFAULT_SAMPLE_RATE * decimation / 1e6);
    audio_gain             = DEFAULT_AUDIO_GAIN;

    currentFrequency = center_freq;

    qDebug() << sample_rate << audio_samp_rate << audio_gain << cut_off << transition << decimation;

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

    std::string ver = gr::version();
    qDebug() << "GNU Radio Version: " + ver;
    qDebug() << "Channel Count: " + QString::number(hackrf_osmo_source->get_num_channels());
    qDebug() << "Center Frequency: " << hackrf_osmo_source->get_center_freq(0) << " Hz";
    qDebug() << "Sample Rate: " << hackrf_osmo_source->get_sample_rate() << " Hz\n";
    qDebug() << "Actual RX Gain: " << hackrf_osmo_source->get_gain() << " dB...";
    qDebug() << "IF Gain: " << hackrf_osmo_source->get_gain("IF", 0) << " dB";
    qDebug() << "BB Gain: " << hackrf_osmo_source->get_gain("BB", 0) << " dB";

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
    return 0;
}

void OsmoDevice::setSampleRate(double sampleRate)
{
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
        case mData:
        {
            auto text = QString(parsedValue.data());
            QStringList parts = text.split(',');

            // Extract the command and value
            QString command = parts.value(0);
            QString valueString = parts.value(1);

            // Convert the value string to uint64_t to handle large values
            // bool conversionOkValue;
            // auto value = valueString.toDouble(&conversionOkValue);
            if (command == "set_ip") {
                customAudioSink->connectToServer(valueString, 5001);
            }
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
