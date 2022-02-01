#include <QCoreApplication>
#include "simpleserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    SimpleServer server;

    return a.exec();
}
