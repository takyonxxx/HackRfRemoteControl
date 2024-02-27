#include <QCoreApplication>
#ifdef Q_OS_MACOS
#include "soapydevice.h"
#endif
#ifdef Q_OS_LINUX
#include "osmodevice.h"
#endif

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    #ifdef Q_OS_MACOS
    auto sdr_device = new SoapyDevice();
    #endif
    #ifdef Q_OS_LINUX
    auto sdr_device = new OsmoDevice();
    #endif

    sdr_device->start();
    sdr_device->wait();

    return a.exec();
}
