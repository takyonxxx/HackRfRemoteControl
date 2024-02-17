#include "sdrdevice.h"
SdrDevice::SdrDevice(QObject *parent):
    QThread(parent)
{

}

SdrDevice::~SdrDevice()
{       
}

void SdrDevice::setFrequency(double frequency)
{
    if (hackrf_soapy_source) {
        hackrf_soapy_source->setFrequency(SOAPY_SDR_RX, 0, frequency);
    }
    if (hackrf_osmo_source) {
        hackrf_osmo_source->set_center_freq(frequency);
    }
}

double SdrDevice::getCenterFrequency() const
{
    if (hackrf_soapy_source)
    {
        return hackrf_soapy_source->getFrequency(SOAPY_SDR_RX, 0);
    }
    if (hackrf_osmo_source)
    {
        return hackrf_osmo_source->get_center_freq();
    }
    return 0;
}

void SdrDevice::setSampleRate(double sampleRate) {
    if (hackrf_soapy_source) {
        hackrf_soapy_source->setSampleRate(SOAPY_SDR_RX, 0, sampleRate);
    }
    if (hackrf_osmo_source) {
        hackrf_osmo_source->set_sample_rate(sampleRate);
    }
}

void SdrDevice::setGain(double gain) {
    if (hackrf_soapy_source) {
        hackrf_soapy_source->setGain(SOAPY_SDR_RX, 0, "IF", gain);
    }
    if (hackrf_osmo_source) {
        hackrf_osmo_source->set_gain(gain);
    }
}

void SdrDevice::run()
{
    tb = gr::make_top_block("HackRf");

    double samp_rate = DEFAULT_SAMPLE_RATE;
    double audio_samp_rate = DEFAULT_AUDIO_SAMPLE_RATE;
    double center_freq = DEFAULT_FREQUENCY;
    double audio_gain = 0.75;

    //    SoapySDR::Kwargs args;
    //    args["driver"] = "hackrf";
    //    args["device"] = "0";


    //    hackrf_soapy_source = SoapySDR::Device::make(args);
    //    if(hackrf_soapy_source)
    //    {
    //         hackrf_soapy_source->setSampleRate(SOAPY_SDR_RX, 0, DEFAULT_SAMPLE_RATE);
    //         hackrf_soapy_source->setFrequency(SOAPY_SDR_RX, 0, DEFAULT_FREQUENCY);
    //         hackrf_soapy_source->setGain(SOAPY_SDR_RX, 0, "IF", 40.0);
    //         hackrf_soapy_source->setGain(SOAPY_SDR_RX, 0, "BB", 40.0);

    //         qDebug() << "Actual IF Gain: " << hackrf_soapy_source->getGain(SOAPY_SDR_RX, 0, "IF") << " dB";
    //         qDebug() << "Actual BB Gain: " << hackrf_soapy_source->getGain(SOAPY_SDR_RX, 0, "BB") << " dB";
    //         qDebug() << "Actual Gain: " << hackrf_soapy_source->getGain(SOAPY_SDR_RX, 0) << " dB";

    //         auto currentFrequency = hackrf_soapy_source->getFrequency(SOAPY_SDR_RX, 0);
    //         qDebug() << currentFrequency << " MHz";
    //    }

    hackrf_osmo_source = osmosdr::source::make("hackrf=0");

    hackrf_osmo_source->set_time_unknown_pps(osmosdr::time_spec_t());
    hackrf_osmo_source->set_sample_rate(samp_rate);
    hackrf_osmo_source->set_center_freq(center_freq, 0);
    hackrf_osmo_source->set_freq_corr(0, 0);
    hackrf_osmo_source->set_gain(0, "AMP", false);
    hackrf_osmo_source->set_if_gain(40, 0);
    hackrf_osmo_source->set_bb_gain(40, 0);
    hackrf_osmo_source->set_antenna("", 0);
    hackrf_osmo_source->set_bandwidth(0, 0);

    std::cout << "Center Frequency: " << hackrf_osmo_source->get_center_freq(0) << " Hz\n";
    std::cout << "Sample Rate: " << hackrf_osmo_source->get_sample_rate() << " Hz\n";
    std::cout << "Actual Bandwidth: " << hackrf_osmo_source->get_bandwidth(0) << " [Hz]...\n";
    std::cout << "Actual RX Gain: " << hackrf_osmo_source->get_gain() << " dB...\n";
    std::cout << "IF Gain: " << hackrf_osmo_source->get_gain("IF", 0) << " dB\n";
    std::cout << "BB Gain: " << hackrf_osmo_source->get_gain("BB", 0) << " dB\n";
    std::cout << "RX Antenna: " << hackrf_osmo_source->get_antenna(0) << '\n';

    gr::top_block_sptr tb = gr::make_top_block("FM Receiver");

    //    gr::filter::rational_resampler_ccc::sptr rational_resampler = gr::filter::rational_resampler_ccc::make(
    //        12, 5, {});
    gr::filter::rational_resampler_base_ccc::sptr rational_resampler = gr::filter::rational_resampler_base_ccc::make(
        12, 5, {});
    // Create the low-pass filter
    auto low_pass_filter = gr::filter::fir_filter_ccf::make(
        50,
        gr::filter::firdes::low_pass(1, samp_rate, DEFAULT_CUT_OFF, DEFAULT_TRANSLITION, gr::fft::window::WIN_HAMMING, 6.76));

    auto multiply_const = gr::blocks::multiply_const_ff::make(audio_gain);

    auto audio_sink = gr::audio::sink::make(audio_samp_rate, "", true);
    gr::analog::quadrature_demod_cf::sptr fm_demod = gr::analog::quadrature_demod_cf::make(1.0);

    tb->connect(fm_demod, 0, multiply_const, 0);
    tb->connect(multiply_const, 0, audio_sink, 0);
    tb->connect(low_pass_filter, 0, rational_resampler, 0);
    tb->connect(rational_resampler, 0, fm_demod, 0);
    tb->connect(hackrf_osmo_source, 0, low_pass_filter, 0);

    tb->run();
    tb->wait();
}
