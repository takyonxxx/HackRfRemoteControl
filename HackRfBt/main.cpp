#include <QCoreApplication>
#include "modulator.h"
#include "hackrfmanager.h"
#include "sdrmanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    HackRfManager hackRfManager;
    SdrManager sdrManager;
    Modulator *mod = new Modulator();

    qDebug() << "Searching Sdr devices...";

    if (HackRfManager::isHackRfAvailable()) {
        qDebug() << "Starting HackRF device.";
        if (!hackRfManager.Open(mod)) {
             qDebug() << "Could not open Hackrf";
        }
        hackRfManager.StartRx();
    }
    else if (rtlsdr_get_device_count() > 0)
    {
        qDebug() << "Starting Rtl device.";
        sdrManager.start();
    }
    else
         qDebug() << "No Sdr devices found.";

    return a.exec();
}
