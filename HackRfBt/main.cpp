#include <QCoreApplication>
#include "osmodevice.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    auto sdr_device = new OsmoDevice();
    sdr_device->start();
    sdr_device->wait();
    return a.exec();
}
