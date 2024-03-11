#ifndef CUSTOMBUFFER_H
#define CUSTOMBUFFER_H
#include "tcpclient.h"
#include <gnuradio/sync_block.h>


class CustomBuffer: public gr::sync_block
{
public:

    typedef std::shared_ptr<CustomBuffer> sptr;
    static sptr make(const std::string& device_name = "");

    CustomBuffer(const std::string& device_name);
    ~CustomBuffer() override;

    void connectToServer(const QString &hostAddress, quint16 port);

private:
    TcpClient *tcpClient{};
    int work(int noutput_items, gr_vector_const_void_star& input_items, gr_vector_void_star& output_items) override;
};

#endif // CUSTOMBUFFER_H
