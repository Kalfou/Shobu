#include <QCoreApplication>

#include "shobuserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ShobuServer server;

    return a.exec();
}
