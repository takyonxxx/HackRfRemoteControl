#include <QCoreApplication>
#include "sdrdevice.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    
    auto sdr_device = new SdrDevice();

    // HackRfManager hackRfManager;
    //  Modulator *mod = new Modulator();

    // qDebug() << "Searching Sdr devices...";

    // if (HackRfManager::isHackRfAvailable()) {
    //     qDebug() << "Starting HackRF device.";
    //     if (!hackRfManager.Open(mod)) {
    //          qDebug() << "Could not open Hackrf";
    //     }
    //     hackRfManager.StartRx();
    // }
    // else
    //      qDebug() << "No HackRf devices found.";

    return a.exec();
}
