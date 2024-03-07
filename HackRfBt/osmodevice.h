#ifndef OSMODEVICE_H
#define OSMODEVICE_H

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

#include <osmosdr/source.h>
#include <message.h>
#include "gattserver.h"
#include "customaudiosink.h"



#define GHZ(x) ((uint64_t)(x) * 1000000000)
#define MHZ(x) ((x) * 1000000)
#define KHZ(x) ((x) * 1000)
#define HZ(x) ((x) * 1)
#define DEFAULT_SAMPLE_RATE             MHZ(20)
#define DEFAULT_AUDIO_SAMPLE_RATE       KHZ(48)
#define DEFAULT_CHANNEL_WIDTH           KHZ(300)
#define DEFAULT_FREQUENCY               MHZ(100)
#define DEFAULT_AUDIO_GAIN              1.0

class OsmoDevice : public QThread
{
    Q_OBJECT
public:
    explicit OsmoDevice(QObject *parent = nullptr);
    ~OsmoDevice();

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
    GattServer *gattServer{};   
    Message message;
    gr::top_block_sptr tb;
    std::shared_ptr<CustomAudioSink> customAudioSink;

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
    bool m_ptt;
    Demod currentDemod;
    FreqMod currentFreqMod;

    QString enumDemodToString(OsmoDevice::Demod demod)
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

    QString enumFreqModToString(OsmoDevice::FreqMod fmod)
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

//     SoapySDR::Kwargs args;
//     args["driver"] = "hackrf";
//     args["device"] = "0";

//     std::string dev = "hackrf=0";
//     std::string stream_args = "";
//     std::vector<std::string> tune_args = {""};
//     std::vector<std::string> settings = {""};

//     hackrf_soapy_source = SoapySDR::Device::make(args);

//     hackrf_soapy_source->setSampleRate(SOAPY_SDR_RX, 0, sample_rate);
//     hackrf_soapy_source->setBandwidth(SOAPY_SDR_RX, 0, 0);
//     hackrf_soapy_source->setFrequency(SOAPY_SDR_RX, 0, center_freq);
//     hackrf_soapy_source->setGain(0, SOAPY_SDR_RX, "AMP", false);
//     hackrf_soapy_source->setGain(SOAPY_SDR_RX, 0, "LNA", std::min(std::max(40.0, 0.0), 40.0));
//     hackrf_soapy_source->setGain(SOAPY_SDR_RX, 0, "VGA", std::min(std::max(40.0, 0.0), 62.0));

//     std::string ver = gr::version();
//     qDebug() << "GNU Radio Version: " + ver;
//     qDebug() << "Center Frequency: " << hackrf_soapy_source->getFrequency(SOAPY_SDR_RX, 0) << " Hz";
//     qDebug() << "Sample Rate: " << hackrf_soapy_source->getSampleRate(SOAPY_SDR_RX, 0) << " Hz\n";
//     qDebug() << "Actual RX Gain: " << hackrf_soapy_source->getGain(SOAPY_SDR_RX, 0) << " dB...";
//     qDebug() << "LNA Gain: " << hackrf_soapy_source->getGain(SOAPY_SDR_RX, 0, "LNA") << " dB";
//     qDebug() << "VGA Gain: " << hackrf_soapy_source->getGain(SOAPY_SDR_RX, 0, "VGA") << " dB";

//     gr::soapy::sink::sptr hackrf_soapy_sink = gr::soapy::sink::make(dev, "fc32", 1, "hackrf=0", "", tune_args, settings);
// }

#endif // OSMODEVICE_H
