#include "serverui.h"
#include "serverlogic.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ServerLogic server;
    int port = 3000;
    server.startServer(port);
    ServerUI window;
    window.show();
    return a.exec();
}
