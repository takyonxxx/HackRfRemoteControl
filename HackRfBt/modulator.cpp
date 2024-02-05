#include "modulator.h"
#include "CustomAudioSink.h"

CustomAudioSink::sptr CustomAudioSink::make(AudioOutputThread* audioOutputThread)
{
    return gnuradio::get_initial_sptr(new CustomAudioSink(audioOutputThread));
}

int CustomAudioSink::work(int noutput_items, gr_vector_const_void_star& input_items, gr_vector_void_star& output_items)
{
    QMutexLocker locker(&mutex);

    // Get the demodulated audio samples from the input_items vector
    const float* in = (const float*)input_items[0];

    // Convert and send the samples to AudioOutputThread
    QByteArray soundBuffer(reinterpret_cast<const char*>(in), noutput_items * sizeof(float));
    audioOutputThread->writeBuffer(soundBuffer);

    // Return the number of output items processed
    return noutput_items;
}

CustomAudioSink::CustomAudioSink(AudioOutputThread* audioOutputThread)
    : gr::block("CustomAudioSink", gr::io_signature::make(1, 1, sizeof(float)), gr::io_signature::make(0, 0, 0)),
    audioOutputThread(audioOutputThread)
{
    // Additional initialization if needed
}

Modulator::Modulator()
{   
    m_sample_count = SAMPLE_COUNT;
    m_sample_rate = 48000;
    audioOutputThread = new AudioOutputThread(nullptr, m_sample_rate);

    // Create GNU Radio flowgraph
    tb = gr::make_top_block("fm_demod_flowgraph");

    // Source block for HackRF (replace with your actual HackRF source)
    hackrf_source = gr::osmosdr::source::make();
    hackrf_source->set_sample_rate(m_sample_rate);
    hackrf_source->set_center_freq(currentFrequency);
    hackrf_source->set_gain(20);  // Set the appropriate gain value

    // FM demodulator block
    fm_demod = gr::analog::quadrature_demod_cf::make(1.0);  // Set the appropriate gain value

    // Custom Audio Sink block
    audio_sink = CustomAudioSink::make(audioOutputThread);

    // Throttle block to control sample rate
    gr::blocks::throttle::sptr throttle = gr::blocks::throttle::make(sizeof(gr_complex), m_sample_rate);

    // Connect the blocks
    tb->connect(hackrf_source, 0, fm_demod, 0);
    tb->connect(fm_demod, 0, throttle, 0);
    tb->connect(throttle, 0, audio_sink, 0);

    tb->run();
}

Modulator::~Modulator()
{
    if (audioOutputThread) {
        delete audioOutputThread;
    }
}

double Modulator::demodulate(double inputSample)
{
    // FM demodulation using phase-locked loop (PLL)
    double phaseIncrement = 2 * PI * IF_FREQUENCY / m_sample_rate;
    double error = inputSample * sin(phase);
    phase += phaseIncrement + DEMOD_GAIN * error;

    return phase;
}

int Modulator::onData(int8_t* buffer, uint32_t length)
{
    m_mutex.lock();

    // FM demodulation
    for (uint32_t i = 0; i < length; ++i) {
        double inputSample = static_cast<double>(buffer[i]) / 128.0; // Assuming 8-bit signed samples
        double demodulatedSample = demodulate(inputSample);

        // Your processing logic for the demodulated sample goes here

        // Example: Pass the demodulated sample to the audio output thread
        if (audioOutputThread) {
            QByteArray soundBuffer(reinterpret_cast<const char*>(&demodulatedSample), sizeof(double));
            audioOutputThread->writeBuffer(soundBuffer);
        }
    }

    m_mutex.unlock();

    return 0;
}
