#include "custombuffer.h"

CustomBuffer::CustomBuffer(const std::string& device_name)
: gr::sync_block(device_name,
                     gr::io_signature::make(1, 1, sizeof(float)),
                     gr::io_signature::make(0, 0, 0))
{
    tcpClient = new TcpClient();
}

CustomBuffer::~CustomBuffer()
{
    if(tcpClient)
    {
        delete tcpClient;
    }
}

void CustomBuffer::connectToServer(const QString &hostAddress, quint16 port)
{
    if(tcpClient)
    {
        tcpClient->connectToServer(hostAddress, port);
    }
}

int CustomBuffer::work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items)
{
        const float* in = (const float*)input_items[0];
        size_t byte_size = noutput_items * sizeof(float);
        // Buffer the audio data
        QByteArray buffer(reinterpret_cast<const char*>(in), byte_size);

        if(tcpClient)
        {
            tcpClient->sendData(buffer);
        }

        return noutput_items;
}

CustomBuffer::sptr CustomBuffer::make(const std::string& device_name)
{
    return std::make_shared<CustomBuffer>(device_name);
}
