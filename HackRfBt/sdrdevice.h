#ifndef SDRDEVICE_H
#define SDRDEVICE_H

#define GHZ(x) ((uint64_t)(x) * 1000000000)
#define MHZ(x) ((x) * 1000000)
#define KHZ(x) ((x) * 1000)
#define DEFAULT_SAMPLE_RATE             MHZ(2)
#define DEFAULT_CHANNEL_WIDTH           KHZ(300)
#define DEFAULT_FREQUENCY               MHZ(100)
#define DEFAULT_AUDIO_SAMPLE_RATE       KHZ(48)
#define DEFAULT_AUDIO_GAIN              0.75

#include <QCoreApplication>
#include <QBuffer>
#include <QDebug>
#include <QThread>

#include <gnuradio/constants.h>
#include <gnuradio/prefs.h>
#include <gnuradio/top_block.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/audio/sink.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fir_filter_blk.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <osmosdr/source.h>
#include <message.h>
#include "gattserver.h"

class SdrDevice : public QThread
{
    Q_OBJECT
public:
    explicit SdrDevice(QObject *parent = nullptr);
    ~SdrDevice();

    void setFrequency(double frequency);
    double getCenterFrequency() const;
    void setSampleRate(double sampleRate);
    double getSampleRate();
    void setGain(double gain);
    typedef enum {
        DEMOD_AM,
        DEMOD_WFM
    } Demod;

    typedef enum {
        HZ,
        KHZ,
        MHZ,
        GHZ
    } FreqMod;

private:
    osmosdr::source::sptr hackrf_osmo_source;
    gr::top_block_sptr tb;
    GattServer *gattServer{};
    Message message;

    int sample_rate ;
    int audio_samp_rate;
    int center_freq;
    int cut_off;
    int transition;
    int decimation;
    int interpolation;
    int resampler_decimation;
    double audio_gain;
    double currentFrequency;

    QString enumToString(SdrDevice::Demod demod)
    {
        switch (demod)
        {
        case SdrDevice::DEMOD_AM:
            return "AM";
        case SdrDevice::DEMOD_WFM:
            return "WFM";
        default:
            return "Unknown";
        }
    }

private slots:
    void onConnectionStatedChanged(bool);
    void onDataReceived(QByteArray);
    void createMessage(uint8_t msgId, uint8_t rw, QByteArray payload, QByteArray *result);
    bool parseMessage(QByteArray *data, uint8_t &command, QByteArray &value, uint8_t &rw);
    void sendCommand(uint8_t command, uint8_t value);
    void sendString(uint8_t command, const QString&);
    void onInfoReceived(QString);

protected:
    void run() override;
};

#endif // SDRDEVICE_H
