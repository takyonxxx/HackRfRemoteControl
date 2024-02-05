#ifndef _MODULATOR_H
#define _MODULATOR_H

#include <QDebug>
#include <mutex>
#include "IHackRFData.h"
#include <audiootputthread.h>
#include <gnuradio/top_block.h>
#include <gnuradio/osmosdr/source.h>
#include <gnuradio/analog/quadrature_demod_cf.h>  // Include the quadrature demodulation block
#include <gnuradio/blocks/throttle.h>

#define BUF_NUM  256
#define BYTES_PER_SAMPLE  2
#define PI 3.1415926535897932384626433832795
#define SAMPLE_COUNT 2048
#define PREF_QUAD_RATE  96000.f

#define GHZ(x) ((uint64_t)(x) * 1000000000)
#define MHZ(x) ((x) * 1000000)
#define KHZ(x) ((x) * 1000)
#define DEFAULT_SAMPLE_RATE             MHZ(8)
#define DEFAULT_FREQUENCY               MHZ(100)

#define BUF_LEN 262144         //hackrf rx buf


class CustomAudioSink : public gr::block
{
public:
    typedef std::shared_ptr<CustomAudioSink> sptr;

    static sptr make(AudioOutputThread* audioOutputThread);
    int work(int noutput_items, gr_vector_const_void_star& input_items, gr_vector_void_star& output_items) override;

private:
    AudioOutputThread* audioOutputThread;
    QMutex mutex;

    CustomAudioSink(AudioOutputThread* audioOutputThread);
};

class Modulator:public IHackRFData
{

private:    
    AudioOutputThread *audioOutputThread{};
    std::mutex m_mutex;   
    uint32_t m_sample_rate;
    size_t m_sample_count;
    uint32_t hackrf_sample;
    const double IF_FREQUENCY = 100000;  // Intermediate frequency for demodulation
    const double DEMOD_GAIN = 0.9;
    double phase = 0.0;
public:
    Modulator();
    ~Modulator();

    gr::top_block_sptr getTopBlock() const { return tb; }
    gr::osmosdr::source::sptr getHackRFSource() const { return hackrf_source; }
    gr::analog::quadrature_demod_cf::sptr getFMDemodulator() const { return fm_demod; }
    CustomAudioSink::sptr getAudioSink() const { return audio_sink; }


    int onData(int8_t* buffer, uint32_t length);
private:
    double demodulate(double inputSample);
    gr::top_block_sptr tb;
    gr::osmosdr::source::sptr hackrf_source;
    gr::analog::quadrature_demod_cf::sptr fm_demod;
    CustomAudioSink::sptr audio_sink;
};

#endif
