#include <QCoreApplication>
#include <QTimer>
#include "sdrdevice.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    
    auto sdr_device = new SdrDevice();
    sdr_device->start();

    const double frequencyIncrement = 0.5e6; // 0.1 MHz in Hz
    double currentFrequency = DEFAULT_FREQUENCY; // Set your initial frequency here

    // Create a timer to handle the frequency change
    QTimer frequencyTimer;

    frequencyTimer.start(100);

    qDebug() << "Timer started";

    QObject::connect(&frequencyTimer, &QTimer::timeout, [&]() {
        currentFrequency = sdr_device->getCenterFrequency();
        qDebug() << currentFrequency;
        sdr_device->setFrequency(currentFrequency + frequencyIncrement);
        sdr_device->start();
    });

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
