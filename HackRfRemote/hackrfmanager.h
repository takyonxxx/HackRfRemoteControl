#ifndef HACKRFMANAGER_H
#define HACKRFMANAGER_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QDebug>
#include <message.h>
#include "audiooutput.h"

#define MHZ(x)                          ((x)*1000*1000)
#define KHZ(x)                          ((x)*1*1000)
#define HZ(x)                          ((x)*1)
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

class HackRfManager : public QThread
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
    } Demod;   

    static int getSamplingBytes(Demod demod) {
        switch (demod) {
        case DEMOD_AM:
            return 1024;
        case DEMOD_WFM:
            return 4196;
        default:
            return -1;
        }
    }

public:
    typedef enum {
        HZ,
        KHZ,
        MHZ,
        GHZ
    } FreqMod;

    void setStop(bool newStop);
    void setDemod(Demod newDemod);

private:
    QQueue<QByteArray> m_bufferQueue;
    AudioOutput *audioOutput{};
    bool m_stop;
    bool m_ptt;
    QMutex mutex;

signals:
    void sendInfo(QString);
protected:
    void run() override;
};

#endif // HACKRFMANAGER_H
