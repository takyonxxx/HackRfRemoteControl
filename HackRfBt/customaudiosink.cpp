#include "customaudiosink.h"

CustomAudioSink::CustomAudioSink(int sampling_rate, const std::string& device_name, bool ok_to_block)
    : gr::sync_block(device_name,
                     gr::io_signature::make(1, 1, sizeof(float)),  // Input signature
                     gr::io_signature::make(0, 0, 0)),
    audioDevice(nullptr)
{
    // Initialize your Qt audio device (replace this with your actual initialization)
    // QAudioFormat format;
    // format.setSampleFormat(QAudioFormat::Float);
    // format.setSampleRate(sampling_rate);
    // format.setChannelCount(1);

    // QAudioDevice outputDevice;

    // for (auto &device : QMediaDevices::audioOutputs()) {
    //     outputDevice = device;
    //     break;
    // }

    // if (outputDevice.isNull()) {
    //     qDebug() << "No valid audio output device found.";
    //     return;
    // }

    // qDebug() << "Default Sound Device: " << outputDevice.description() << sampling_rate;

    // m_audioOutput.reset(new QAudioSink(outputDevice, format));
    // m_audioOutput->setBufferSize(4098);
    // audioDevice = m_audioOutput->start();

    tcpClient = new TcpClient();
}

CustomAudioSink::~CustomAudioSink()
{
    // Cleanup resources if needed
    // if (audioDevice) {
    //     audioDevice->close();
    // }

    if(tcpClient)
    {
        delete tcpClient;
    }
}

void CustomAudioSink::connectToServer(const QString &hostAddress, quint16 port)
{
    tcpClient->connectToServer(hostAddress, port);
}

int CustomAudioSink::work(int noutput_items, gr_vector_const_void_star& input_items, gr_vector_void_star& output_items)
{
    // if (!audioDevice) {
    //     return 0;  // Return if the audio device is not initialized
    // }
    const float* in = (const float*)input_items[0];
    size_t byte_size = noutput_items * sizeof(float);

    // Buffer the audio data
    QByteArray buffer(reinterpret_cast<const char*>(in), byte_size);

    // Write the audio data to the Qt audio device
    // qint64 bytesWritten = audioDevice->write(buffer);
    // if (bytesWritten < 0) {
    //     qDebug() << "Error writing to audio device:" << audioDevice->errorString();
    //     // Handle the error (e.g., return an error code or throw an exception)
    // }

    if(tcpClient)
        tcpClient->sendData(buffer);

    // return bytesWritten / sizeof(float);
    return noutput_items;
}

CustomAudioSink::sptr CustomAudioSink::make(int sampling_rate, const std::string& device_name, bool ok_to_block)
{
    return std::make_shared<CustomAudioSink>(sampling_rate, device_name, ok_to_block);
}
