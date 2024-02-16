#ifndef SDRDEVICE_H
#define SDRDEVICE_H

#define GHZ(x) ((uint64_t)(x) * 1000000000)
#define MHZ(x) ((x) * 1000000)
#define KHZ(x) ((x) * 1000)
#define DEFAULT_SAMPLE_RATE             MHZ(8)
#define DEFAULT_BANDWITH                KHZ(240)
#define DEFAULT_FREQUENCY               MHZ(100)
#define DEFAULT_AUDIO_SAMPLE_RATE       44100

#include <QCoreApplication>
#include <QBuffer>
#include <QDebug>
#include <QThread>

#include <gnuradio/top_block.h>
#include <gnuradio/blocks/multiply_const.h>
#include <gnuradio/blocks/multiply.h>
#include <gnuradio/blocks/throttle.h>
#include <gnuradio/filter/rational_resampler_base.h>
#include <gnuradio/analog/sig_source.h>
#include <gnuradio/qtgui/freq_sink_c.h>
#include <gnuradio/audio/sink.h>
#include <gnuradio/analog/quadrature_demod_cf.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/fir_filter_blk.h>
#include <gnuradio/math.h>
#include <osmosdr/source.h>

 #include <SoapySDR/Device.hpp>
 #include <SoapySDR/Formats.hpp>
 #include <SoapySDR/Errors.hpp>

// #include <audiootput.h>

class SdrDevice : public QThread
{
    Q_OBJECT
public:
    explicit SdrDevice(QObject *parent = nullptr);
    ~SdrDevice();

    void setFrequency(double frequency);
    double getCenterFrequency() const;
    void setSampleRate(double sampleRate);
    void setGain(double gain);

private:
    // SoapySDR::Device *hackrf_source;
    // AudioOutput* audioOutput{};

    gr::top_block_sptr tb;
    gr::analog::quadrature_demod_cf::sptr fm_demod;
    osmosdr::source::sptr hackrf_source;
    bool m_stop;
protected:
    void run() override;
};

#endif // SDRDEVICE_H

//    auto filter_taps = low_pass_filter->taps();
// Convert the filter_taps to gr_complex for rational_resampler_base_ccc
//    std::vector<gr_complex> complex_taps(filter_taps.size(), gr_complex(0.0, 0.0));
//    for (size_t i = 0; i < filter_taps.size(); ++i) {
//        complex_taps[i] = gr_complex(filter_taps[i], 0.0);
//    }
//    auto rational_resampler = gr::filter::rational_resampler_base_ccc::make(12, 5, complex_taps);
