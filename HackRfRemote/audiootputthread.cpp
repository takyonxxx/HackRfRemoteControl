#include "audiootputthread.h"
#include <QDebug>
#include <QBuffer>

AudioOutputThread::AudioOutputThread(QObject *parent):
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

    m_format.setSampleFormat(QAudioFormat::Int16);
    m_format.setSampleRate(22050);
    m_format.setChannelCount(1);

    m_audioOutput.reset(new QAudioSink(outputDevice, m_format));
    io = m_audioOutput->start();
    queue = new QQueue<QByteArray>();
    qDebug() << "Default Sound Device: " << outputDevice.description();
}

AudioOutputThread::~AudioOutputThread()
{
    delete queue;
    delete mutex;
}

void AudioOutputThread::handleAudioOutputStateChanged(QAudio::State newState)
{
    if (newState == QAudio::StoppedState) {
    } else if (newState == QAudio::ActiveState) {
    }
}

void AudioOutputThread::stop()
{
    m_abort = true;
}

void AudioOutputThread::writeBuffer(const QByteArray &buffer)
{   
    QMutexLocker locker(mutex);
    if (!m_abort)
    {       
        io->write(buffer.data(), buffer.size());
    }
}

//const int sampleRate = 22050;
//const int frequency = 440;  // Frequency of the sine wave (in Hz)
//const int duration = 1;
//QByteArray soundBuffer;
//const int bufferSize = sampleRate * duration * sizeof(short);
//soundBuffer.resize(bufferSize);
//generateSineWave(reinterpret_cast<short*>(soundBuffer.data()), bufferSize, sampleRate, frequency);
//io->write(soundBuffer.data(), soundBuffer.size());

void AudioOutputThread::generateSineWave(short *buffer, int bufferSize, int sampleRate, int frequency)
{
    const double twoPi = 2.0 * 3.141592653589793238462643383279502884197169399375105820974944;

    for (int i = 0; i < bufferSize / sizeof(short); ++i)
    {
        double time = static_cast<double>(i) / sampleRate;
        buffer[i] = static_cast<short>(32767.0 * sin(twoPi * frequency * time));
    }
}
