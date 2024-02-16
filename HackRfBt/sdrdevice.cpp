#include "sdrdevice.h"
SdrDevice::SdrDevice(QObject *parent):
    QThread(parent)
{
    // SoapySDR::Kwargs args;
    // args["driver"] = "hackrf";
    // args["device"] = "0";

    // hackrf_source = SoapySDR::Device::make(args);
    // hackrf_source->setSampleRate(SOAPY_SDR_RX, 0, DEFAULT_SAMPLE_RATE);
    // hackrf_source->setFrequency(SOAPY_SDR_RX, 0, DEFAULT_FREQUENCY);
    // hackrf_source->setGain(SOAPY_SDR_RX, 0, "IF", 20.0); // Adjust the gain parameter as needed
    // auto rates = hackrf_source->getSampleRateRange(SOAPY_SDR_RX, 0);
}

SdrDevice::~SdrDevice()
{       
}

void SdrDevice::setFrequency(double frequency)
{
    if (hackrf_source) {
        // hackrf_source->setFrequency(SOAPY_SDR_RX, 0, frequency);
        hackrf_source->set_center_freq(frequency);
    }
}

double SdrDevice::getCenterFrequency() const
{
    if (hackrf_source)
    {
        return hackrf_source->get_center_freq();
    }
    return 0;
}

void SdrDevice::setSampleRate(double sampleRate) {
    if (hackrf_source) {
        // hackrf_source->setSampleRate(SOAPY_SDR_RX, 0, sampleRate);
        hackrf_source->set_sample_rate(sampleRate);
    }
}

void SdrDevice::setGain(double gain) {
    if (hackrf_source) {
        // hackrf_source->setGain(SOAPY_SDR_RX, 0, "IF", gain);
        hackrf_source->set_gain(gain);
    }
}

void SdrDevice::run()
{
    tb = gr::make_top_block("HackRfBlock");

    double samp_rate = 8e6;
    double freq = 100e6;
    double signal_freq = 300e3;
    double audio_gain = 0.2;

    hackrf_source = osmosdr::source::make("hackrf=0");

    hackrf_source->set_time_unknown_pps(osmosdr::time_spec_t());
    hackrf_source->set_sample_rate(samp_rate);
    hackrf_source->set_center_freq(freq, 0);
    hackrf_source->set_freq_corr(0, 0);
    hackrf_source->set_gain(10, 0);
    hackrf_source->set_if_gain(40, 0);
    hackrf_source->set_bb_gain(40, 0);
    hackrf_source->set_antenna("", 0);
    hackrf_source->set_bandwidth(0, 0);

    std::cout << "Center Frequency: " << hackrf_source->get_center_freq(0) << " Hz\n";
    std::cout << "Sample Rate: " << hackrf_source->get_sample_rate() << " Hz\n";
    std::cout << "Actual Bandwidth: " << hackrf_source->get_bandwidth(0) << " [Hz]...\n";
    std::cout << "Actual RX Gain: " << hackrf_source->get_gain() << " dB...\n";
    std::cout << "IF Gain: " << hackrf_source->get_gain("IF", 0) << " dB\n";
    std::cout << "BB Gain: " << hackrf_source->get_gain("BB", 0) << " dB\n";
    std::cout << "RX Antenna: " << hackrf_source->get_antenna(0) << '\n';

    gr::top_block_sptr tb = gr::make_top_block("FM Receiver");

    // Create the low-pass filter
    auto low_pass_filter = gr::filter::fir_filter_ccf::make(
        50,
        gr::filter::firdes::low_pass(1, samp_rate, 75e3, 25000, gr::filter::firdes::WIN_HAMMING, 6.76));
    auto filter_taps = low_pass_filter->taps();

    // Convert the filter_taps to gr_complex for rational_resampler_base_ccc
    std::vector<gr_complex> complex_taps(filter_taps.size(), gr_complex(0.0, 0.0));
    for (size_t i = 0; i < filter_taps.size(); ++i) {
        complex_taps[i] = gr_complex(filter_taps[i], 0.0);
    }

    // Create the rational resampler with the obtained filter taps
    auto rational_resampler = gr::filter::rational_resampler_base_ccc::make(12, 5, complex_taps);

    gr::blocks::multiply_cc::sptr blocks_multiply_xx_0 = gr::blocks::multiply_cc::make(1);

    auto multiply_const = gr::blocks::multiply_const_ff::make(audio_gain);

    auto audio_sink = gr::audio::sink::make(44100, "", true);

    auto sig_source = gr::analog::sig_source_c::make(samp_rate, gr::analog::GR_COS_WAVE, signal_freq, 1, 0, 0);

    gr::analog::quadrature_demod_cf::sptr fm_demod = gr::analog::quadrature_demod_cf::make(1.0);    

    tb->connect(fm_demod, 0, multiply_const, 0);
    tb->connect(sig_source, 0, blocks_multiply_xx_0, 1);
    tb->connect(multiply_const, 0, audio_sink, 0);
    tb->connect(blocks_multiply_xx_0, 0, low_pass_filter, 0);
    tb->connect(low_pass_filter, 0, rational_resampler, 0);
    tb->connect(hackrf_source, 0, blocks_multiply_xx_0, 0);
    tb->connect(rational_resampler, 0, fm_demod, 0);

    auto currentFrequency = getCenterFrequency();
    qDebug() << currentFrequency / 1000000.0 << " MHz";

    tb->run();
    tb->wait();
}
