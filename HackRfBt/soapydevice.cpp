#include "soapydevice.h"

SoapyDevice::SoapyDevice(QObject *parent):
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

#ifdef Q_OS_MACOS
    SoapySDR::Kwargs args;
    args["driver"] = "hackrf";
    args["device"] = "0";

    std::string dev = "hackrf=0";
    std::string stream_args = "";
    std::vector<std::string> tune_args = {""};
    std::vector<std::string> settings = {""};

    hackrf_soapy_source = SoapySDR::Device::make(args);

    hackrf_soapy_source->setSampleRate(SOAPY_SDR_RX, 0, sample_rate);
    hackrf_soapy_source->setBandwidth(SOAPY_SDR_RX, 0, 0);
    hackrf_soapy_source->setFrequency(SOAPY_SDR_RX, 0, center_freq);
    hackrf_soapy_source->setGain(0, SOAPY_SDR_RX, "AMP", false);
    hackrf_soapy_source->setGain(SOAPY_SDR_RX, 0, "LNA", std::min(std::max(40.0, 0.0), 40.0));
    hackrf_soapy_source->setGain(SOAPY_SDR_RX, 0, "VGA", std::min(std::max(40.0, 0.0), 62.0));

    std::string ver = gr::version();
    qDebug() << "GNU Radio Version: " + ver;
    qDebug() << "Center Frequency: " << hackrf_soapy_source->getFrequency(SOAPY_SDR_RX, 0) << " Hz";
    qDebug() << "Sample Rate: " << hackrf_soapy_source->getSampleRate(SOAPY_SDR_RX, 0) << " Hz\n";
    qDebug() << "Actual RX Gain: " << hackrf_soapy_source->getGain(SOAPY_SDR_RX, 0) << " dB...";
    qDebug() << "LNA Gain: " << hackrf_soapy_source->getGain(SOAPY_SDR_RX, 0, "LNA") << " dB";
    qDebug() << "VGA Gain: " << hackrf_soapy_source->getGain(SOAPY_SDR_RX, 0, "VGA") << " dB";

    gr::soapy::sink::sptr hackrf_soapy_sink = gr::soapy::sink::make(dev, "fc32", 1, "hackrf=0", "", tune_args, settings);
#endif
}

SoapyDevice::~SoapyDevice()
{
}

void SoapyDevice::setFrequency(double frequency)
{

}

double SoapyDevice::getCenterFrequency() const
{
    return 0;
}

void SoapyDevice::setSampleRate(double sampleRate)
{
}

double SoapyDevice::getSampleRate()
{
    return sample_rate;
}

void SoapyDevice::setGain(double gain)
{

}

void SoapyDevice::onConnectionStatedChanged(bool state)
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

void SoapyDevice::onDataReceived(QByteArray data)
{
    uint8_t parsedCommand;
    uint8_t rw;
    QByteArray parsedValue;
    auto parsed = parseMessage(&data, parsedCommand, parsedValue, rw);

    if(!parsed)return;

    bool ok;
    int value =  parsedValue.toHex().toInt(&ok, 16);
}

void SoapyDevice::createMessage(uint8_t msgId, uint8_t rw, QByteArray payload, QByteArray *result)
{
    uint8_t buffer[MaxPayload+8] = {'\0'};
    uint8_t command = msgId;

    int len = message.create_pack(rw , command , payload, buffer);

    for (int i = 0; i < len; i++)
    {
        result->append(buffer[i]);
    }
}

bool SoapyDevice::parseMessage(QByteArray *data, uint8_t &command, QByteArray &value, uint8_t &rw)
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

void SoapyDevice::sendCommand(uint8_t command, uint8_t value)
{
    // Convert the uint32_t value to a QByteArray
    QByteArray payload;
    payload.append(reinterpret_cast<const char*>(&value), sizeof(value));

    // Create the message and send it
    QByteArray sendData;
    createMessage(command, mWrite, payload, &sendData);
    gattServer->writeValue(sendData);
}

void SoapyDevice::sendString(uint8_t command, const QString& value)
{
    QByteArray sendData;
    QByteArray bytedata;
    bytedata = value.toLocal8Bit();
    createMessage(command, mWrite, bytedata, &sendData);
    gattServer->writeValue(sendData);
}

void SoapyDevice::onInfoReceived(QString info)
{
    qDebug() << info;
}

void SoapyDevice::run()
{
    tb = gr::make_top_block("HackRf");

    gr::filter::rational_resampler_ccf::sptr resampler = gr::filter::rational_resampler_ccf::make(interpolation, resampler_decimation);
    gr::analog::quadrature_demod_cf::sptr quad_demod = gr::analog::quadrature_demod_cf::make(1.0);
    auto low_pass_filter = gr::filter::fir_filter_fff::make(
        6,
        gr::filter::firdes::low_pass(1, sample_rate, cut_off, transition, gr::fft::window::WIN_HAMMING, 6.76));
    auto audio_sink = gr::audio::sink::make(audio_samp_rate, "", true);
    auto multiply_const = gr::blocks::multiply_const_ff::make(audio_gain);
    tb->start();
}
