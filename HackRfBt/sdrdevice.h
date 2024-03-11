#ifndef SDRDEVICE_H
#define SDRDEVICE_H

#include <QCoreApplication>
#include <QBuffer>
#include <QDebug>
#include <QThread>

#include <gnuradio/constants.h>
#include <gnuradio/prefs.h>
#include <gnuradio/top_block.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/audio/sink.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fir_filter_blk.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/gr_complex.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/soapy/source.h>

#include <message.h>
#include "gattserver.h"
#include "custombuffer.h"

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>

//#include <osmosdr/source.h>

#define _GHZ(x) ((uint64_t)(x) * 1000000000)
#define _MHZ(x) ((x) * 1000000)
#define _KHZ(x) ((x) * 1000)
#define _HZ(x) ((x) * 1)
#define DEFAULT_SAMPLE_RATE             _MHZ(20)
#define DEFAULT_AUDIO_SAMPLE_RATE       _KHZ(48)
#define DEFAULT_CHANNEL_WIDTH           _KHZ(300)
#define DEFAULT_FREQUENCY               _MHZ(100)
#define DEFAULT_AUDIO_GAIN              1.0

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

class SdrDevice : public QThread
{
    Q_OBJECT
public:
    explicit SdrDevice(QObject *parent = nullptr);
    ~SdrDevice();

    void setFrequency(double frequency);
    double getCenterFrequency() const;   

private:
    gr::soapy::source::sptr hackrf_soapy_source;
//  osmosdr::source::sptr hackrf_osmo_source;
//#endif

    GattServer *gattServer{};   
    Message message;
    gr::top_block_sptr tb;
    std::shared_ptr<CustomBuffer> customBuffer;

    int sample_rate ;
    int audio_samp_rate;
    int cut_off;
    int transition;
    int decimation;
    int interpolation;
    int resampler_decimation;
    double audio_gain;
    double currentFrequency;
    bool m_ptt;
    Demod currentDemod;
    FreqMod currentFreqMod;

    QString enumDemodToString(Demod demod)
    {
        switch (demod)
        {
        case DEMOD_AM:
            return "AM";
        case DEMOD_WFM:
            return "WFM";
        default:
            return "Unknown";
        }
    }

    QString enumFreqModToString(FreqMod fmod)
    {
        switch (fmod)
        {
        case HZ:
            return "Hz";
        case KHZ:
            return "KHz";
        case MHZ:
            return "MHz";
        case GHZ:
            return "GHz";
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
