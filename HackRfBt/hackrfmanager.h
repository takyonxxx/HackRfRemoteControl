#ifndef HACKRFMANAGER_H
#define HACKRFMANAGER_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <gattserver.h>
#include <message.h>
#include "audiootputthread.h"
#include "receiver/receiver.hh"
#include "udpclient.h"
#include "tcpclient.h"

class HackRfManager : public QObject
{
    Q_OBJECT
public:
    explicit HackRfManager(QObject *parent = nullptr);
    ~HackRfManager();

    void setAbort(bool newAbort);

public:
    typedef enum {
        DEMOD_AM,
        DEMOD_WFM,
        DEMOD_NFM,
        DEMOD_USB,
        DEMOD_LSB,
        DEMOD_CW,
        DEMOD_BPSK31
    } Demod;

public:
    typedef enum {
        HZ,
        KHZ,
        MHZ,
        GHZ
    } FreqMod;

private:
    void createMessage(uint8_t msgId, uint8_t rw, QByteArray payload, QByteArray *result);
    bool parseMessage(QByteArray *data, uint8_t &command, QByteArray &value, uint8_t &rw);

    void sendCommand(uint8_t command, uint8_t value);
    void sendString(uint8_t command, const QString&);

    unsigned int sampleRate;
    qint64       tunerFrequency;
    int          m_HiCutFreq;
    int          m_LowCutFreq;
    unsigned int fftSize;
    unsigned int fftrate;

    DemodulatorCtrl::Demod currentDemod;
    FreqMod currentFreqMod;
    AudioOutputThread *audioOutputThread{};
    UdpClient *udpClient{};
    TcpClient *tcpClient{};

    GattServer *gattServer{};
    QQueue<QByteArray> bufferQueue;

    QAudioSink *m_audioOutput;
    QAudioFormat m_format;
    QBuffer m_audioOutputIODevice;
    QQueue<QByteArray> audioQueue;

    Message message;
    bool m_ptt;
    QMutex mutex;
    bool m_abort;

private slots:
    void onConnectionStatedChanged(bool);
    void onDataReceived(QByteArray);
    void onInfoReceived(QString);
    void onReceiverStarted();
    void onReceiverStopped();
    void fftTimeout();
    void onFilterChanged();
    void onBufferProcessed(const sdr::Buffer<int16_t> &buffer);

    QString enumToString(HackRfManager::Demod demod)
    {
        switch (demod)
        {
        case HackRfManager::DEMOD_AM:
            return "AM";
        case HackRfManager::DEMOD_WFM:
            return "WFM";
        case HackRfManager::DEMOD_NFM:
            return "NFM";
        case HackRfManager::DEMOD_USB:
            return "USB";
        case HackRfManager::DEMOD_LSB:
            return "LSB";
        case HackRfManager::DEMOD_CW:
            return "CW";
        case HackRfManager::DEMOD_BPSK31:
            return "BPSK31";
        default:
            return "Unknown";
        }
    }

protected:    
    Receiver *m_Receiver{};
    DemodulatorCtrl *m_Demodulator{};
};

#endif // HACKRFMANAGER_H
