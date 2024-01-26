#ifndef HACKRFMANAGER_H
#define HACKRFMANAGER_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QDebug>
#include <message.h>
#include "audiootputthread.h"

const int sampleRate = 22050;
const int channelCount = 1;

#define MHZ(x)                          ((x)*1000*1000)
#define KHZ(x)                          ((x)*1*1000)
#define DEFAULT_SAMPLE_RATE             MHZ(2.5)
#define DEFAULT_FREQUENCY               MHZ(100)
#define DEFAULT_FREQUENCY_CORRECTION	60 //ppm
#define DEFAULT_FFT_SIZE                8192 * 4
#define DEFAULT_FFT_RATE                25 //Hz
#define DEFAULT_FREQ_STEP               5 //kHz
#define DEFAULT_AUDIO_GAIN              50
#define DEFAULT_HICUT_FREQ              KHZ(100)
#define MAX_FFT_SIZE                    DEFAULT_FFT_SIZE
#define RESET_FFT_FACTOR                -72

class HackRfManager : public QObject
{
    Q_OBJECT
public:
    explicit HackRfManager(QObject *parent = nullptr);
    ~HackRfManager();

    void setBuffer(const QByteArray& buffer);
    void playSound();
    void setPtt(bool newPtt);

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
    AudioOutputThread *audioOutputThread{};
    bool m_stop;
    bool m_ptt;
    QMutex mutex;

signals:
    void sendInfo(QString);
};

#endif // HACKRFMANAGER_H
