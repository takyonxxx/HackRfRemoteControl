#ifndef CUSTOMAUDIOSINK_H
#define CUSTOMAUDIOSINK_H
#include <QCoreApplication>

#include <QAudioSink>
#include <QMediaDevices>
#include <QAudioFormat>
#include <QAudioOutput>

#include <gnuradio/sync_block.h>

class CustomAudioSink : public gr::sync_block
{
public:
    typedef std::shared_ptr<CustomAudioSink> sptr;
    static sptr make(int sampling_rate, const std::string& device_name = "", bool ok_to_block = true);

    CustomAudioSink(int sampling_rate, const std::string& device_name, bool ok_to_block);
    ~CustomAudioSink() override;

private:
    QIODevice* audioDevice;
    QScopedPointer<QAudioSink> m_audioOutput;
    int work(int noutput_items, gr_vector_const_void_star& input_items, gr_vector_void_star& output_items) override;
};

#endif // CUSTOMAUDIOSINK_H
