#include <QCoreApplication>
#include "hackrfmanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    HackRfManager hackRfManager;
    return a.exec();
}
