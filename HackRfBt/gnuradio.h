#ifndef GNURADIO_H
#define GNURADIO_H

#define GHZ(x) ((uint64_t)(x) * 1000000000)
#define MHZ(x) ((x) * 1000000)
#define KHZ(x) ((x) * 1000)
#define DEFAULT_SAMPLE_RATE             MHZ(8)
#define DEFAULT_FREQUENCY               MHZ(100)
#define DEFAULT_AUDIO_SAMPLE_RATE       48000

#include <QObject>
#include <QDebug>
#include <gnuradio/top_block.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/audio/sink.h>
#include <gnuradio/logger.h>
#include "osmosdr/source.h"

// #include <SoapySDR/Device.hpp>
// #include <SoapySDR/Formats.hpp>
// #include <SoapySDR/Errors.hpp>

class GnuRadioManager : public QObject
{
    Q_OBJECT
public:
    explicit GnuRadioManager();
    ~GnuRadioManager();

    gr::top_block_sptr getTopBlock() const;
    gr::analog::quadrature_demod_cf::sptr getFMDemodulator() const;

private:
    gr::top_block_sptr tb;  
    gr::analog::quadrature_demod_cf::sptr fm_demod;
    osmosdr::source::sptr hackrf_source;
    // SoapySDR::Device *hackrf_source;
    // gr::block_sptr hackrf_source;
    // SoapySDR::Kwargs args;
    // args["driver"] = "hackrf";
    // args["device"] = "0";
    // hackrf_source = SoapySDR::Device::make(args);
    // hackrf_source->setSampleRate(SOAPY_SDR_RX, 0, DEFAULT_SAMPLE_RATE);
    // hackrf_source->setFrequency(SOAPY_SDR_RX, 0, DEFAULT_FREQUENCY);
    // hackrf_source->setGain(SOAPY_SDR_RX, 0, "IF", 20.0); // Adjust the gain parameter as needed
};

#endif // GNURADIO_H
