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
    auto sdr_device = new OsmoDevice();
    sdr_device->start();
    sdr_device->wait();
    return a.exec();
}
