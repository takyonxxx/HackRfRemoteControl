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
    hackrf_source = osmosdr::source::make("hackrf=0");
    hackrf_source->set_sample_rate(DEFAULT_SAMPLE_RATE);
    hackrf_source->set_center_freq(DEFAULT_FREQUENCY);
    // hackrf_source->set_gain_mode(true);
    hackrf_source->set_gain(14.0, 0);
    hackrf_source->set_gain(40.0, "IF", 0);
    hackrf_source->set_gain(40.0, "BB", 0);

    std::cout << "Center Frequency: " << hackrf_source->get_center_freq(0) << " Hz\n";
    std::cout << "Sample Rate: " << hackrf_source->get_sample_rate() << " Hz\n";
    std::cout << "Actual Bandwidth: " << hackrf_source->get_bandwidth(0) << " [Hz]...\n";
    std::cout << "Actual RX Gain: " << hackrf_source->get_gain() << " dB...\n";
    std::cout << "IF Gain: " << hackrf_source->get_gain("IF", 0) << " dB\n";
    std::cout << "BB Gain: " << hackrf_source->get_gain("BB", 0) << " dB\n";
    std::cout << "RX Antenna: " << hackrf_source->get_antenna(0) << '\n';

    tb = gr::make_top_block("HackRfBlock");
    // Create FM demodulation block
    gr::analog::quadrature_demod_cf::sptr fm_demod = gr::analog::quadrature_demod_cf::make(1.0);
    gr::blocks::throttle::sptr throttle = gr::blocks::throttle::make(4, DEFAULT_SAMPLE_RATE);

    // dec 50
    // gain 1
    // samplerate 8M
    // Cutoff freq: 75K
    // Transition Width: 25K
    // Hamming
    // beta 6.76

       // Create audio sink block
    gr::audio::sink::sptr audio_sink = gr::audio::sink::make(DEFAULT_AUDIO_SAMPLE_RATE, "", true);

    tb->connect(hackrf_source, 0, fm_demod, 0);
    tb->connect(fm_demod, 0, throttle, 0);
    tb->connect(throttle, 0, audio_sink, 0);

    auto currentFrequency = getCenterFrequency();
    qDebug() << currentFrequency / 1000000.0;
    tb->run();
    tb->wait();
}
