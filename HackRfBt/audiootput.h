#ifndef AUIOOUTPUTTHREAD_H
#define AUIOOUTPUTTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QBuffer>
#include <QAudioSink>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QIODevice>

#define DEFAULT_AUDIO_SAMPLE_RATE       44100
#define DEFAULT_CHANNEL_COUNT           1

class AudioOutput: public QObject
{
    Q_OBJECT
public:
    explicit AudioOutput(QObject *parent = nullptr,
                         int sampleFormat = DEFAULT_AUDIO_SAMPLE_RATE,
                         int channelCount = DEFAULT_CHANNEL_COUNT);
    ~AudioOutput();
    void stop();
    void writeBuffer(const QByteArray &buffer);
    void setSampleRate(int sampleRate)
    {
        m_format.setSampleRate(sampleRate);
    }

private slots:
    void handleAudioOutputStateChanged(QAudio::State newState);

private:

    QMutex *mutex{};
    bool m_abort {false};
    QQueue<QByteArray> *queue{};

    QAudioFormat m_format;
    QScopedPointer<QAudioSink> m_audioOutput;
    QIODevice *io;
    QQueue<QByteArray> m_audioQueue;
    QMutex m_mutex;
};

#endif // AUIOOUTPUTTHREAD_H


