#ifndef SOAPYDEVICE_H

#include <QCoreApplication>
#include <QBuffer>
#include <QDebug>
#include <QThread>

#include <message.h>
#include "gattserver.h"

#include <gnuradio/constants.h>
#include <gnuradio/prefs.h>
#include <gnuradio/top_block.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/filter/rational_resampler.h>
#include <gnuradio/audio/sink.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fir_filter_blk.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/messages/msg_queue.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/gr_complex.h>
#include <gnuradio/soapy/sink.h>

#ifdef Q_OS_MACOS
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Logger.hpp>
#endif

class SoapyDevice : public QThread
{
    Q_OBJECT
public:
    explicit SoapyDevice(QObject *parent = nullptr);
    ~SoapyDevice();

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
#ifdef Q_OS_MACOS
    SoapySDR::Device *hackrf_soapy_source;
#endif
    GattServer *gattServer{};
    Message message;
    gr::top_block_sptr tb;

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

    QString enumToString(SoapyDevice::Demod demod)
    {
        switch (demod)
        {
        case SoapyDevice::DEMOD_AM:
            return "AM";
        case SoapyDevice::DEMOD_WFM:
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

#endif // SOAPYDEVICE_H
