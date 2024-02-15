#include "sdrdevice.h"
SdrDevice::SdrDevice(QObject *parent):
    QThread(parent)
{
    SoapySDR::Kwargs args;
    args["driver"] = "hackrf";
    args["device"] = "0";

    hackrf_source = SoapySDR::Device::make(args);

    if (!hackrf_source) {
        // Handle initialization failure
        std::cerr << "Failed to initialize HackRF source." << std::endl;
        return;
    }

    hackrf_source->setSampleRate(SOAPY_SDR_RX, 0, DEFAULT_SAMPLE_RATE);
    hackrf_source->setFrequency(SOAPY_SDR_RX, 0, DEFAULT_FREQUENCY);
    hackrf_source->setGain(SOAPY_SDR_RX, 0, "IF", 20.0); // Adjust the gain parameter as needed
    auto rates = hackrf_source->getSampleRateRange(SOAPY_SDR_RX, 0);

    audioOutput = std::make_unique<AudioOutput>(this, 48000);

    this->start();
}

SdrDevice::~SdrDevice()
{
    delete hackrf_source;
}

void SdrDevice::run()
{
    try {

        SoapySDR::Stream *rxStream = hackrf_source->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
        hackrf_source->activateStream(rxStream, 0, 0, 0);

        // Create buffer for storing samples
        const size_t numSamples = 1024; // Adjust as needed
        std::vector<std::complex<int16_t>> buffer(numSamples);

        // Read samples from the stream
        int flags = 0; // Adjust flags as needed
        long long timeNs; // Time in nanoseconds, you can ignore it for now

        while (true)
        {
            size_t numBytesRead = hackrf_source->readStream(rxStream, reinterpret_cast<void **>(buffer.data()), numSamples, flags, timeNs);
            if (numBytesRead == 0) {
                continue;
            }
            // FM demodulation using a simple phase-locked loop (PLL)
            std::vector<float> audioBuffer(numSamples);
            float phase = 0.0;
            for (size_t i = 0; i < numSamples; ++i)
            {
                // Compute phase difference
                float phaseDiff = std::arg(buffer[i]);

                // Adjust phase to keep it within [-pi, pi]
                while (phaseDiff < -M_PI)
                    phaseDiff += 2 * M_PI;
                while (phaseDiff > M_PI)
                    phaseDiff -= 2 * M_PI;

                // Update phase using the phase difference
                phase += phaseDiff;

                // Perform FM demodulation
                audioBuffer[i] = phase * (DEFAULT_SAMPLE_RATE / (2.0 * M_PI));
            }

            // Convert float audio samples to QByteArray
            QByteArray audioData(reinterpret_cast<const char*>(audioBuffer.data()), audioBuffer.size() * sizeof(float));
            audioOutput->writeBuffer(audioData);

            QThread::msleep(10);
        }

        // Deactivate the stream when done
        hackrf_source->deactivateStream(rxStream, 0, 0);
        hackrf_source->closeStream(rxStream);

        // Cleanup
        SoapySDR::Device::unmake(hackrf_source);
    } catch (const std::exception& e) {
        // Handle exceptions and log error message
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
