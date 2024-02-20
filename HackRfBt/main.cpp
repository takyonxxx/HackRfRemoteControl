#include <QCoreApplication>
#include "sdrdevice.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    auto sdr_device = new SdrDevice();
    sdr_device->start();
    sdr_device->wait();

    return a.exec();
}
