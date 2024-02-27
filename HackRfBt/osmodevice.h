#ifndef OSMODEVICE_H
#define OSMODEVICE_H

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
#include <gnuradio/sync_block.h>
#include <gnuradio/messages/msg_queue.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/gr_complex.h>
#include <message.h>
#include "gattserver.h"

#ifdef Q_OS_LINUX
#include <osmosdr/source.h>

class BufferBlock : public gr::block
{
public:
    typedef std::shared_ptr<BufferBlock> sptr;

    BufferBlock();
    void set_input_buffer(const std::vector<float>& input_buffer);

private:
    std::vector<float> input_buffer;

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items) override;
};
#endif

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

#ifdef Q_OS_LINUX
    osmosdr::source::sptr hackrf_osmo_source;
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

    QString enumToString(OsmoDevice::Demod demod)
    {
        switch (demod)
        {
        case OsmoDevice::DEMOD_AM:
            return "AM";
        case OsmoDevice::DEMOD_WFM:
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

#endif // OSMODEVICE_H
