#ifndef HACKRFMANAGER_H
#define HACKRFMANAGER_H

#include <QObject>
#include <QDebug>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <cmath>
#include <message.h>
#include <gattserver.h>
#include <audiootputthread.h>
#include "hackrf.h"
#include "IHackRFData.h"
#include "tcpclient.h"

#define STANDBY_MODE 0
#define RX_MODE      1
#define TX_MODE      2

#define GHZ(x) ((uint64_t)(x) * 1000000000)
#define MHZ(x) ((x) * 1000000)
#define KHZ(x) ((x) * 1000)
#define DEFAULT_SAMPLE_RATE             MHZ(8)
#define DEFAULT_FREQUENCY               MHZ(100)
#define DEFAULT_FREQUENCY_CORRECTION	60 //ppm
#define DEFAULT_FFT_SIZE                8192 * 4
#define DEFAULT_FFT_RATE                25 //Hz
#define DEFAULT_FREQ_STEP               5 //kHz
#define DEFAULT_AUDIO_GAIN              50
#define DEFAULT_HICUT_FREQ              KHZ(100)
#define MAX_FFT_SIZE                    DEFAULT_FFT_SIZE
#define RESET_FFT_FACTOR                -72

using namespace std;


class HackRfManager : public QObject
{
    Q_OBJECT
public:
    explicit HackRfManager(QObject *parent = nullptr);
    ~HackRfManager();

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

    int m_device_mode;    // mode: 0=standby 1=Rx 2=Tx
    FILE* fd = NULL;

    int HackRFRxCallback(int8_t* buffer, uint32_t length);
    int HackRFTxCallback(int8_t* buffer, uint32_t length);

    static bool isHackRfAvailable() {
        // Initialize HackRF library
        hackrf_device* device = nullptr;
        int result = hackrf_init();

        if (result != HACKRF_SUCCESS) {
            // HackRF library initialization failed
            return false;
        }

        // Open HackRF device
        result = hackrf_open(&device);

        // Close HackRF device and cleanup
        hackrf_close(device);
        hackrf_exit();

        // Return true if opening the device was successful
        return result == HACKRF_SUCCESS;
    }

    bool Open(IHackRFData *handler);
    void onFrequencyChangeTimer();
    bool handle_error(int status, const char * format, ...);

//    int hackRF_tx_callback(hackrf_transfer* transfer);
//    int hackRF_rx_callback(hackrf_transfer* transfer);
    bool set_sample_rate( double rate );
    bool set_center_freq( double fc_hz );

    virtual bool StartRx();
    bool stop_Rx();
    virtual bool StartTx();
    bool stop_Tx();

private slots:
    void onConnectionStatedChanged(bool);
    void onDataReceived(QByteArray);
    void onInfoReceived(QString);
    void createMessage(uint8_t msgId, uint8_t rw, QByteArray payload, QByteArray *result);
    bool parseMessage(QByteArray *data, uint8_t &command, QByteArray &value, uint8_t &rw);
    void sendCommand(uint8_t command, uint8_t value);
    void sendString(uint8_t command, const QString&);

private:
    IHackRFData *mRxHandler;
    IHackRFData *mTxHandler;
    GattServer *gattServer{};
    AudioOutputThread *audioOutputThread{};
    Message message;

    bool m_is_initialized;
    int sampleRate;

    hackrf_device *_device{};
    double previous_phase = 0.0;
    double previous_amplitude = 0.0;
    double currentFrequency;
    double deviation = 50e3;      // 50 kHz deviation
    double modulationIndex;

    double fm_phase;
    double fm_deviation;
    uint32_t hackrf_sample = 2000000;
    float * output;

    bool m_ptt;

    Demod currentDemod;
    FreqMod currentFreqMod;
    TcpClient *tcpClient{};

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

    double calculateFrequency(double baseFrequency, double deviation, double modulationIndex) {
        return baseFrequency + deviation * std::sin(modulationIndex);
    }

};
#endif // HACKRFMANAGER_H
