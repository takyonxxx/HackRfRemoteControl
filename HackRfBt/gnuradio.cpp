#include "gnuradio.h"

GnuRadioManager::GnuRadioManager()
{
//    gr::logger_ptr logger = gr::logger_get_logger("YourLoggerName");
//    gr::logger_set_level(logger, "INFO");

    tb = gr::make_top_block("fm_demod_flowgraph");

    try {

        // Create OsmoSDR source block
        hackrf_source = osmosdr::source::make("hackrf=0");
        hackrf_source->set_sample_rate(DEFAULT_SAMPLE_RATE);
        hackrf_source->set_center_freq(DEFAULT_FREQUENCY);
        hackrf_source->set_gain(0, "IF", 20.0); // Adjust the gain parameter as needed

        // Create FM demodulation block
        gr::analog::quadrature_demod_cf::sptr fm_demod = gr::analog::quadrature_demod_cf::make(1.0);
        fm_demod->set_gain(40.0);
        // Create Throttle block
        gr::blocks::throttle::sptr throttle = gr::blocks::throttle::make(4, DEFAULT_SAMPLE_RATE);
        gr::audio::sink::sptr audio_sink = gr::audio::sink::make(DEFAULT_AUDIO_SAMPLE_RATE, "", true);

        // Connect the blocks
        tb->connect(hackrf_source, 0, fm_demod, 0);
        tb->connect(fm_demod, 0, throttle, 0);
        tb->connect(throttle, 0, audio_sink, 0);

        // Run the flowgraph
        tb->run();
    } catch (const std::exception& e) {
        // Handle exceptions and log error message
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

GnuRadioManager::~GnuRadioManager()
{

}

gr::top_block_sptr GnuRadioManager::getTopBlock() const
{
    return tb;
}

gr::analog::quadrature_demod_cf::sptr GnuRadioManager::getFMDemodulator() const
{
    return fm_demod;
}
