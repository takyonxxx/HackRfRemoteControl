#include "sdrdevice.h"

BufferBlock::BufferBlock() : gr::block("BufferBlock",
                gr::io_signature::make(1, 1, sizeof(float)),  // Input signature
                gr::io_signature::make(0, 0, 0))  // Output signature (empty)
                // gr::io_signature::make(1, 1, sizeof(float)))  // Output signature
{
}

void BufferBlock::set_input_buffer(const std::vector<float>& input_buffer)
{
    this->input_buffer = input_buffer;
}

int BufferBlock::general_work(int noutput_items,
                              gr_vector_int& ninput_items,
                              gr_vector_const_void_star& input_items,
                              gr_vector_void_star& output_items)
{
    // Ensure that there is input data available
    if (input_items[0] == 0) {
        return 0;
    }

    // Get pointers to input buffer
    const float* in = (const float*)input_items[0];
    // Get the byte size of the input buffer
    size_t byte_size = noutput_items * sizeof(float);
    // Print the byte size
    std::cout << "Byte Size of Input Buffer: " << byte_size << " bytes" << std::endl;

    // Process input as needed

    // Return the number of input items consumed
    return noutput_items;
}

SdrDevice::SdrDevice(QObject *parent):
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

    std::string ver = gr::version();
    qDebug() << "GNU Radio Version: " + ver;
    qDebug() << "Center Frequency: " << hackrf_osmo_source->get_center_freq(0) << " Hz";
    qDebug() << "Sample Rate: " << hackrf_osmo_source->get_sample_rate() << " Hz\n";
    qDebug() << "Actual RX Gain: " << hackrf_osmo_source->get_gain() << " dB...";
    qDebug() << "IF Gain: " << hackrf_osmo_source->get_gain("IF", 0) << " dB";
    qDebug() << "BB Gain: " << hackrf_osmo_source->get_gain("BB", 0) << " dB";

    // gattServer = GattServer::getInstance();
    // if (gattServer)
    // {
    //     qDebug() << "Starting gatt service";
    //     QObject::connect(gattServer, &GattServer::connectionState, this, &SdrDevice::onConnectionStatedChanged);
    //     QObject::connect(gattServer, &GattServer::dataReceived, this, &SdrDevice::onDataReceived);
    //     QObject::connect(gattServer, &GattServer::sendInfo, this, &SdrDevice::onInfoReceived);
    //     gattServer->startBleService();
    // }
}

SdrDevice::~SdrDevice()
{       
}

void SdrDevice::setFrequency(double frequency)
{
    if (hackrf_osmo_source) {
        hackrf_osmo_source->set_center_freq(frequency);
        currentFrequency = getCenterFrequency();
    }
}

double SdrDevice::getCenterFrequency() const
{   
    if (hackrf_osmo_source)
    {
        return hackrf_osmo_source->get_center_freq();
    }
    return 0;
}

void SdrDevice::setSampleRate(double sampleRate)
{
    if (hackrf_osmo_source) {
        hackrf_osmo_source->set_sample_rate(sampleRate);
        sample_rate = getSampleRate();
    }
}

double SdrDevice::getSampleRate()
{
    if (hackrf_osmo_source) {
        return hackrf_osmo_source->get_sample_rate();
    }
    return sample_rate;
}

void SdrDevice::setGain(double gain)
{
    if (hackrf_osmo_source)
    {
        hackrf_osmo_source->set_if_gain(gain, 0);
        hackrf_osmo_source->set_bb_gain(gain, 0);
    }
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
    gr::analog::quadrature_demod_cf::sptr quad_demod = gr::analog::quadrature_demod_cf::make(1.0);
    auto low_pass_filter = gr::filter::fir_filter_fff::make(
        6,
        gr::filter::firdes::low_pass(1, sample_rate, cut_off, transition, gr::fft::window::WIN_HAMMING, 6.76));
    auto audio_sink = gr::audio::sink::make(audio_samp_rate, "", true);
    auto multiply_const = gr::blocks::multiply_const_ff::make(audio_gain);

    BufferBlock::sptr buffer_block = std::make_shared<BufferBlock>();

    tb->connect(hackrf_osmo_source, 0, resampler, 0);
    tb->connect(resampler, 0, quad_demod, 0);
    tb->connect(quad_demod, 0, low_pass_filter, 0);
    tb->connect(low_pass_filter, 0, multiply_const, 0);
    // tb->connect(multiply_const, 0, buffer_block, 0);
    tb->connect(multiply_const, 0, audio_sink, 0);
    tb->start();
    // Stop the flow graph (never reached in this example)
    // tb->stop();
    // tb->wait();
}
