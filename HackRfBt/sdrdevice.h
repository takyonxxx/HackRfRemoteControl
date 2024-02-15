#ifndef SDRDEVICE_H
#define SDRDEVICE_H

#define GHZ(x) ((uint64_t)(x) * 1000000000)
#define MHZ(x) ((x) * 1000000)
#define KHZ(x) ((x) * 1000)
#define DEFAULT_SAMPLE_RATE             MHZ(8)
#define DEFAULT_FREQUENCY               MHZ(100)
#define DEFAULT_AUDIO_SAMPLE_RATE       48000

#include <QCoreApplication>
#include <QBuffer>
#include <QDebug>
#include <QThread>

#include <gnuradio/top_block.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/audio/sink.h>
#include <gnuradio/logger.h>

#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>

#include <audiootput.h>


class SdrDevice : public QThread
{
    Q_OBJECT
public:
    explicit SdrDevice(QObject *parent = nullptr);
    ~SdrDevice();

    void setFrequency(double frequency);
    void setSampleRate(double sampleRate);
    void setGain(double gain);
    std::vector<SoapySDR::Range> getSampleRateRange() const;
    std::vector<double> listSampleRates() const;

private:   
    SoapySDR::Device *hackrf_source;
    std::unique_ptr<AudioOutput> audioOutput;
    gr::top_block_sptr tb;
    gr::analog::quadrature_demod_cf::sptr fm_demod;
    //osmosdr::source::sptr hackrf_source;
protected:
    void run() override;


    // gr::block_sptr hackrf_source;

    // SoapySDR::Kwargs args;
    // args["driver"] = "hackrf";
    // args["device"] = "0";
    // hackrf_source = SoapySDR::Device::make(args);
    // hackrf_source->setSampleRate(SOAPY_SDR_RX, 0, DEFAULT_SAMPLE_RATE);
    // hackrf_source->setFrequency(SOAPY_SDR_RX, 0, DEFAULT_FREQUENCY);
    // hackrf_source->setGain(SOAPY_SDR_RX, 0, "IF", 20.0); // Adjust the gain parameter as needed

    // Create OsmoSDR source block
    //hackrf_source = osmosdr::source::make("hackrf=0");
    //hackrf_source->set_sample_rate(DEFAULT_SAMPLE_RATE);
    //hackrf_source->set_center_freq(DEFAULT_FREQUENCY);
    //hackrf_source->set_gain(0, "IF", 20.0); // Adjust the gain parameter as needed

    //tb = gr::make_top_block("HackRfBlock");
    //qDebug() << tb->alias();
    //// Create FM demodulation block
    //gr::analog::quadrature_demod_cf::sptr fm_demod = gr::analog::quadrature_demod_cf::make(1.0);
    //fm_demod->set_gain(40.0);
    //// Create Throttle block
    //gr::blocks::throttle::sptr throttle = gr::blocks::throttle::make(4, DEFAULT_SAMPLE_RATE);
    //gr::audio::sink::sptr audio_sink = gr::audio::sink::make(DEFAULT_AUDIO_SAMPLE_RATE, "", true);

    //// Connect the blocks
    //tb->connect(hackrf_source, 0, fm_demod, 0);
    //tb->connect(fm_demod, 0, throttle, 0);
    //tb->connect(throttle, 0, audio_sink, 0);

    //// Run the flowgraph
    //tb->run();
};

#endif // SDRDEVICE_H
