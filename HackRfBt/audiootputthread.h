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

#define DEFAULT_AUDIO_SAMPLE_RATE       48000

class AudioOutputThread: public QObject
{
    Q_OBJECT
public:
    explicit AudioOutputThread(QObject *parent = nullptr, int sampleFormat = DEFAULT_AUDIO_SAMPLE_RATE);
    ~AudioOutputThread();
    void stop();
    void writeBuffer(const QByteArray &buffer);

private slots:
    void handleAudioOutputStateChanged(QAudio::State newState);
    void generateSineWave(short *buffer, int bufferSize, int sampleRate, int frequency);

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
