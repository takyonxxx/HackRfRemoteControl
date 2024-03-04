#ifndef HACKRFMANAGER_H
#define HACKRFMANAGER_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QDebug>
#include <message.h>
#include "audiooutput.h"

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
        DEMOD_NFM,
        DEMOD_USB,
        DEMOD_LSB,
        DEMOD_CW,
        DEMOD_BPSK31
    } Demod;

    static int getSamplingRate(Demod demod) {
        switch (demod) {
        case DEMOD_AM:
            return 8012;  // Example: 8 kHz for AM
        case DEMOD_WFM:
            return 23148; // Example: 22.05 kHz for WFM
        case DEMOD_NFM:
            return 12500;  // Example: 12.5 kHz for NFM
        case DEMOD_USB:
            return 8012;  // Example: 8 kHz for USB/LSB
        case DEMOD_LSB:
            return 8012;  // Example: 8 kHz for USB/LSB
        case DEMOD_CW:
            return 8012;  // Example: 8 kHz for CW
        case DEMOD_BPSK31:
            return 8012; // Example: 11.025 kHz for BPSK31
        default:
            return -1;    // Indicate an error
        }
    }

    static int getSamplingBytes(Demod demod) {
        switch (demod) {
        case DEMOD_AM:
            return 842;
        case DEMOD_WFM:
            return 2428;
        case DEMOD_NFM:
            return 1312;
        case DEMOD_USB:
            return 842;
        case DEMOD_LSB:
            return 842;
        case DEMOD_CW:
            return 842;
        case DEMOD_BPSK31:
            return 842;
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
