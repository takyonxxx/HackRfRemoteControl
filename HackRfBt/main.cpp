#include <QCoreApplication>
#include "sdrmanager.h"
#include "hackrfmanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SdrManager sdrManager;
    HackRfManager hackRfManager;
    qDebug() << "Searching Sdr devices...";

    if (HackRfManager::isHackRfAvailable()) {
        qDebug() << "Starting HackRF device.";
        hackRfManager.start();
    }
    if (rtlsdr_get_device_count() > 0)
    {
        qDebug() << "Starting Rtl device.";
        sdrManager.start();
    }
    else
         qDebug() << "No Sdr devices found.";

    return a.exec();
}
