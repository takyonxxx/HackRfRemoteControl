#include <QCoreApplication>
//#include "sdrmanager.h"
#include "hackrfmanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
//    SdrManager sdrManager;
    HackRfManager hackRfManager;
    return a.exec();
}
