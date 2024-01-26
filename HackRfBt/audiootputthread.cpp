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
//    qreal initialVolume = QAudio::convertVolume(m_audioOutput->volume(), QAudio::LinearVolumeScale,
//                                                QAudio::LogarithmicVolumeScale);

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

void AudioOutputThread::writeBuffer(const sdr::RawBuffer &buffer)
{   
    QMutexLocker locker(mutex);
    if (!m_abort)
    {
        QByteArray soundBuffer(buffer.data(), buffer.bytesLen());
        io->write(soundBuffer.data(), soundBuffer.size());
    }
}
