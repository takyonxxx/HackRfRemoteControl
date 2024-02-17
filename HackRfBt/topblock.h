#ifndef TOPBLOCK_H
#define TOPBLOCK_H
#include <cmath>
#include <stdint.h>

#include <gnuradio/top_block.h>
#include <osmosdr/source.h>
#include <gnuradio/blocks/stream_to_vector.h>
#include <gnuradio/fft/fft_vcc.h>
#include <gnuradio/blocks/complex_to_mag_squared.h>
#include <gnuradio/filter/single_pole_iir_filter_ff.h>
#include <gnuradio/blocks/nlog10_ff.h>

class TopBlock : public gr::top_block
{
public:
    TopBlock(double centre_freq_1, double centre_freq_2, double sample_rate, double fft_width, double bandwidth1, double bandwidth2,
             double step, unsigned int avg_size, double spread, double threshold, double ptime) : gr::top_block("Top Block"),
        vector_length(sample_rate/fft_width),

        source(osmosdr::source::make()), /* OsmoSDR Source */
        stv(gr::blocks::stream_to_vector::make(sizeof(float)*2, vector_length)), /* Stream to vector */
        /* Based on the logpwrfft (a block implemented in python) */
        fft(gr::fft::fft_vcc::make(vector_length, true, window, false, 1)),
        ctf(gr::blocks::complex_to_mag_squared::make(vector_length)),
        iir(gr::filter::single_pole_iir_filter_ff::make(1.0, vector_length))
    {
        /* Set up the OsmoSDR Source */
        source->set_sample_rate(sample_rate);
        source->set_center_freq(centre_freq_1);
        source->set_freq_corr(0.0);
        source->set_gain_mode(false);
        source->set_gain(10.0);
        source->set_if_gain(20.0);

        /* Set up the connections */
//        connect(source, 0, stv, 0);
//        connect(stv, 0, fft, 0);
//        connect(fft, 0, ctf, 0);
//        connect(ctf, 0, iir, 0);
//        connect(iir, 0, lg, 0);
    }

private:

    size_t vector_length;
    std::vector<float> window;

    osmosdr::source::sptr source;
    gr::blocks::stream_to_vector::sptr stv;
    gr::fft::fft_vcc::sptr fft;
    gr::blocks::complex_to_mag_squared::sptr ctf;
    gr::filter::single_pole_iir_filter_ff::sptr iir;
    gr::blocks::nlog10_ff::sptr lg;
};
#endif // TOPBLOCK_H
