#ifndef HACKRFMANAGER_H
#define HACKRFMANAGER_H

#include <QObject>
#include <QDebug>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <message.h>
#include <gattserver.h>
#include "hackrf.h"

#define STANDBY_MODE 0
#define RX_MODE      1
#define TX_MODE      2

#define GHZ(x) ((uint64_t)(x) * 1000000000)
#define MHZ(x) ((x) * 1000000)
#define KHZ(x) ((x) * 1000)
#define DEFAULT_SAMPLE_RATE             MHZ(20)
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

    int m_device_mode;    // mode: 0=standby 1=Rx 2=Tx
    FILE* fd = NULL;

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

    void start();
    void open();
    void handle_error(int status, const char * format, ...);
    static int _hackRF_rx_callback(hackrf_transfer* transfer);
    int hackRF_rx_callback(hackrf_transfer* transfer);
    static int _hackRF_tx_callback(hackrf_transfer* transfer);
    int hackRF_tx_callback(hackrf_transfer* transfer);
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

private:
    GattServer *gattServer{};
    Message message;

    bool m_is_initialized;
    int sampleRate;

    hackrf_device *_device{};
    double previous_phase = 0.0;
    double previous_amplitude = 0.0;
    double centerFrequency;
    double deviation = 50e3;      // 50 kHz deviation
    double modulationIndex;

    double calculateFrequency(double baseFrequency, double deviation, double modulationIndex) {
        return baseFrequency + deviation * std::sin(modulationIndex);
    }

};
#endif // HACKRFMANAGER_H
