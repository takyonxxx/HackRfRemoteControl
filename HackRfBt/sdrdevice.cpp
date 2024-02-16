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
    double channel_freq = 99.7e6;
    double audio_gain = 0.2;

    hackrf_source = osmosdr::source::make("hackrf=0");

    hackrf_source->set_time_unknown_pps(osmosdr::time_spec_t());
    hackrf_source->set_sample_rate(DEFAULT_SAMPLE_RATE);
    hackrf_source->set_center_freq(DEFAULT_FREQUENCY, 0);
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

    // Blocks
    gr::filter::rational_resampler_ccc_sptr rational_resampler_xxx_0 =
        gr::filter::rational_resampler_ccc::make(12, 5, std::vector<float>(), 0.0);

//    gr::filter::fir_filter_ccf_sptr low_pass_filter_0 =
//        gr::filter::fir_filter_ccf::make(50, gr::firdes::low_pass(1, 8000000, 75e3, 25000, gr::filter::window::WIN_HAMMING, 6.76));

    auto  low_pass_filter_0 = gr::filter::firdes::low_pass(1, DEFAULT_SAMPLE_RATE, 75e3, 25000, gr::fft::window::WIN_HAMMING, 6.76);
    gr::filter::kernel::fir_filter_fff low_pass_filter(low_pass_filter_0);

    gr::blocks::multiply_cc::sptr blocks_multiply_xx_0 = gr::blocks::multiply_cc::make(0.2);

    gr::audio::sink::sptr audio_sink = gr::audio::sink::make(DEFAULT_AUDIO_SAMPLE_RATE, "", true);

    gr::analog::sig_source_c::sptr analog_sig_source_x_0 = gr::analog::sig_source_c::make(
        DEFAULT_SAMPLE_RATE, gr::analog::GR_COS_WAVE, DEFAULT_FREQUENCY - channel_freq, 1, 0, 0
        );

    gr::analog::quadrature_demod_cf::sptr fm_demod = gr::analog::quadrature_demod_cf::make(1.0);

    // Connections
    tb->connect(fm_demod, 0, blocks_multiply_xx_0, 0);
    tb->connect(analog_sig_source_x_0, 0, low_pass_filter, 0);
    tb->connect(blocks_multiply_xx_0, 0, audio_sink, 0);
//    tb->connect(low_pass_filter_0, 0, rational_resampler_xxx_0, 0);
//    tb->connect(rational_resampler_xxx_0, 0, analog_fm_demod_cf_0, 0);

    tb->run();
    tb->wait();

//    tb = gr::make_top_block("HackRfBlock");
//    // Create FM demodulation block
//    gr::analog::quadrature_demod_cf::sptr fm_demod = gr::analog::quadrature_demod_cf::make(1.0);
//    gr::blocks::throttle::sptr throttle = gr::blocks::throttle::make(4, DEFAULT_SAMPLE_RATE);

//    // dec 50
//    // gain 1
//    // samplerate 8M
//    // Cutoff freq: 75K
//    // Transition Width: 25K
//    // Hamming
//    // beta 6.76

//       // Create audio sink block
//    gr::audio::sink::sptr audio_sink = gr::audio::sink::make(DEFAULT_AUDIO_SAMPLE_RATE, "", true);

//    tb->connect(hackrf_source, 0, fm_demod, 0);
//    tb->connect(fm_demod, 0, throttle, 0);
//    tb->connect(throttle, 0, audio_sink, 0);

//    auto currentFrequency = getCenterFrequency();
//    qDebug() << currentFrequency / 1000000.0;
//    tb->run();
//    tb->wait();
}
