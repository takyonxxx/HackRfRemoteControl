#include "audiooutput.h"
#include <QDebug>
#include <QBuffer>

AudioOutput::AudioOutput(QObject *parent, int sampleFormat):
    QObject(parent)
{
    QAudioDevice outputDevice;

    for (auto &device : QMediaDevices::audioOutputs()) {
        outputDevice = device;
        break;
    }

    if (outputDevice.isNull()) {
        qDebug() << "No valid audio output device found.";
        return;
    }

    m_format.setSampleFormat(QAudioFormat::Float);
    m_format.setSampleRate(sampleFormat);
    m_format.setChannelCount(1);

    m_audioOutput.reset(new QAudioSink(outputDevice, m_format));
    m_audioOutput->setBufferSize(1024 * 512);
    audioDevice = m_audioOutput->start();

    mutex = new QMutex;

    qDebug() << "Default Sound Device: " << outputDevice.description();
}

AudioOutput::~AudioOutput()
{
    if (audioDevice) {
        audioDevice->close();
    }
    delete mutex;
}

void AudioOutput::handleAudioOutputStateChanged(QAudio::State newState)
{
    if (newState == QAudio::StoppedState) {
    } else if (newState == QAudio::ActiveState) {
    }
}

void AudioOutput::stop()
{
    m_abort = true;
}

void AudioOutput::writeBuffer(const QByteArray &buffer)
{
    if (!m_abort && audioDevice && audioDevice->isOpen())
    {
        QMutexLocker locker(mutex);
        audioDevice->write(buffer);
    }
}

void AudioOutput::generateSineWave(short *buffer, int bufferSize, int sampleRate, int frequency)
{
    const double twoPi = 2.0 * 3.141592653589793238462643383279502884197169399375105820974944;

    for (int i = 0; i < bufferSize / sizeof(short); ++i)
    {
        double time = static_cast<double>(i) / sampleRate;
        buffer[i] = static_cast<short>(32767.0 * sin(twoPi * frequency * time));
    }
}
