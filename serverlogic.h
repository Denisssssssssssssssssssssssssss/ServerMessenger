#ifndef SERVERLOGIC_H
#define SERVERLOGIC_H

#include "logger.h"

#include <QTcpServer>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTcpSocket>

class ServerLogic : public QTcpServer
{
    Q_OBJECT

private:
    QHash<int, QTcpSocket*> userSockets;
    QSqlDatabase database;

private slots:
    void onNewConnection();

public:
    ServerLogic(QObject *parent = nullptr);
    void startServer(int port);

};

#endif // SERVERLOGIC_H
