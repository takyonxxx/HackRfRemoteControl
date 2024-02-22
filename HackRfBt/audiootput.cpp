#include "audiootput.h"
#include <QDebug>
#include <QBuffer>

AudioOutput::AudioOutput(QObject *parent, int sampleFormat, int channelCount):
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
    m_format.setSampleRate(sampleFormat);
    m_format.setChannelCount(channelCount);

    m_audioOutput.reset(new QAudioSink(outputDevice, m_format));
    io = m_audioOutput->start();
    queue = new QQueue<QByteArray>();

    qDebug() << "Default Sound Device: " << outputDevice.description() << sampleFormat << channelCount;
}

AudioOutput::~AudioOutput()
{
    delete queue;
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
    QMutexLocker locker(mutex);
    if (!m_abort)
    {
        io->write(buffer.data(), buffer.size());
    }
}
