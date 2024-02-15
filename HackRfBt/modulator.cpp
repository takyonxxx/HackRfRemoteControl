#include "modulator.h"

Modulator::Modulator()
{
    m_sample_count = SAMPLE_COUNT;
    audioOutputThread = new AudioOutput();
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
    // for (uint32_t i = 0; i < length; ++i) {
    //     double inputSample = static_cast<double>(buffer[i]) / 128.0; // Assuming 8-bit signed samples
    //     double demodulatedSample = demodulate(inputSample);

    //     if (audioOutputThread) {
    //         QByteArray soundBuffer(reinterpret_cast<const char*>(&demodulatedSample), sizeof(double));
    //         audioOutputThread->writeBuffer(soundBuffer);
    //     }
    // }

    m_mutex.unlock();

    return 0;
}
