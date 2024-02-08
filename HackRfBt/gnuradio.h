#ifndef GNURADIO_H
#define GNURADIO_H

// #include <gnuradio/top_block.h>
// #include <gnuradio/osmosdr/source.h>
// #include <gnuradio/analog/quadrature_demod_cf.h>  // Include the quadrature demodulation block
// #include <gnuradio/blocks/throttle.h>

// class CustomAudioSink : public gr::block
// {
// public:
//     typedef std::shared_ptr<CustomAudioSink> sptr;

//     static sptr make(AudioOutputThread* audioOutputThread);
//     int work(int noutput_items, gr_vector_const_void_star& input_items, gr_vector_void_star& output_items) override;

// private:
//     AudioOutputThread* audioOutputThread;
//     QMutex mutex;

//     CustomAudioSink(AudioOutputThread* audioOutputThread);
// };



// gr::top_block_sptr getTopBlock() const { return tb; }
// gr::osmosdr::source::sptr getHackRFSource() const { return hackrf_source; }
// gr::analog::quadrature_demod_cf::sptr getFMDemodulator() const { return fm_demod; }
// CustomAudioSink::sptr getAudioSink() const { return audio_sink; }

// gr::top_block_sptr tb;
// gr::osmosdr::source::sptr hackrf_source;
// gr::analog::quadrature_demod_cf::sptr fm_demod;
// CustomAudioSink::sptr audio_sink;



// CustomAudioSink::sptr CustomAudioSink::make(AudioOutputThread* audioOutputThread)
// {
//     return gnuradio::get_initial_sptr(new CustomAudioSink(audioOutputThread));
// }

// int CustomAudioSink::work(int noutput_items, gr_vector_const_void_star& input_items, gr_vector_void_star& output_items)
// {
//     QMutexLocker locker(&mutex);

//     // Get the demodulated audio samples from the input_items vector
//     const float* in = (const float*)input_items[0];

//     // Convert and send the samples to AudioOutputThread
//     QByteArray soundBuffer(reinterpret_cast<const char*>(in), noutput_items * sizeof(float));
//     audioOutputThread->writeBuffer(soundBuffer);

//     // Return the number of output items processed
//     return noutput_items;
// }

// CustomAudioSink::CustomAudioSink(AudioOutputThread* audioOutputThread)
//     : gr::block("CustomAudioSink", gr::io_signature::make(1, 1, sizeof(float)), gr::io_signature::make(0, 0, 0)),
//     audioOutputThread(audioOutputThread)
// {
//     // Additional initialization if needed
// }


// // Create GNU Radio flowgraph
// tb = gr::make_top_block("fm_demod_flowgraph");

// // Source block for HackRF (replace with your actual HackRF source)
// hackrf_source = gr::osmosdr::source::make();
// hackrf_source->set_sample_rate(m_sample_rate);
// hackrf_source->set_center_freq(currentFrequency);
// hackrf_source->set_gain(20);  // Set the appropriate gain value

// // FM demodulator block
// fm_demod = gr::analog::quadrature_demod_cf::make(1.0);  // Set the appropriate gain value

// // Custom Audio Sink block
// audio_sink = CustomAudioSink::make(audioOutputThread);

// // Throttle block to control sample rate
// gr::blocks::throttle::sptr throttle = gr::blocks::throttle::make(sizeof(gr_complex), m_sample_rate);

// // Connect the blocks
// tb->connect(hackrf_source, 0, fm_demod, 0);
// tb->connect(fm_demod, 0, throttle, 0);
// tb->connect(throttle, 0, audio_sink, 0);

// tb->run();

#endif // GNURADIO_H
