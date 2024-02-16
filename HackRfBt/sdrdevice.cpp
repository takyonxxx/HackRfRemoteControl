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
    double audio_gain = 0.75;

//    SoapySDR::Kwargs args;
//    args["driver"] = "hackrf";
//    args["device"] = "0";

//    auto hackrf_source = SoapySDR::Device::make(args);
//    hackrf_source->setSampleRate(SOAPY_SDR_RX, 0, DEFAULT_SAMPLE_RATE);
//    hackrf_source->setFrequency(SOAPY_SDR_RX, 0, DEFAULT_FREQUENCY);
//    hackrf_source->setGain(SOAPY_SDR_RX, 0, "AMP", false);
//    hackrf_source->setGain(SOAPY_SDR_RX, 0, "LNA", std::min(std::max(40.0, 0.0), 40.0));
//    hackrf_source->setGain(SOAPY_SDR_RX, 0, "VGA", std::min(std::max(40.0, 0.0), 62.0));

    hackrf_source = osmosdr::source::make("hackrf=0");

    hackrf_source->set_time_unknown_pps(osmosdr::time_spec_t());
    hackrf_source->set_sample_rate(samp_rate);
    hackrf_source->set_center_freq(freq, 0);
    hackrf_source->set_freq_corr(0, 0);
    hackrf_source->set_gain(0, 0);
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

    gr::filter::rational_resampler_base_ccc::sptr rational_resampler = gr::filter::rational_resampler_base_ccc::make(
        12, 5, {});

    // Create the low-pass filter
    auto low_pass_filter = gr::filter::fir_filter_ccf::make(
        50,
        gr::filter::firdes::low_pass(1, samp_rate, 75e3, 25000, gr::fft::window::WIN_HAMMING, 6.76));

    auto multiply_const = gr::blocks::multiply_const_ff::make(audio_gain);

    auto audio_sink = gr::audio::sink::make(44100, "", true);    

    gr::analog::quadrature_demod_cf::sptr fm_demod = gr::analog::quadrature_demod_cf::make(1.0);    

    tb->connect(fm_demod, 0, multiply_const, 0);
    tb->connect(multiply_const, 0, audio_sink, 0);
    tb->connect(low_pass_filter, 0, rational_resampler, 0);
    tb->connect(rational_resampler, 0, fm_demod, 0);
    tb->connect(hackrf_source, 0, low_pass_filter, 0);

    auto currentFrequency = getCenterFrequency();
    qDebug() << currentFrequency / 1000000.0 << " MHz";

    tb->run();
    tb->wait();
}
