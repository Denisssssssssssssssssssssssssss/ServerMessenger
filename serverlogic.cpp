#include "serverlogic.h"

ServerLogic::ServerLogic(QObject *parent) : QTcpServer(parent)
{
    connect(this, &ServerLogic::newConnection, this, &ServerLogic::onNewConnection);
    database = QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName(QDir::homePath() + "/MessengerData.db");
    if (!database.open())
    {
        qCritical() << "Could not connect to database:" << database.lastError().text();
        exit(1);
    }
    Logger::getInstance()->logToFile("Server is running");
}

//Обработка подключений и запросов от клиентов
void ServerLogic::onNewConnection()
{
    QTcpSocket *clientSocket = this->nextPendingConnection();
    qintptr socketId = clientSocket->socketDescriptor();
    QString logMessage = QString("New connection. Client socket descriptor: %1").arg(socketId);
    Logger::getInstance()->logToFile(logMessage);
    connect(clientSocket, &QTcpSocket::readyRead, this, [this, clientSocket]()
    {

    });
}

//Запуск сервера на определенном порте
void ServerLogic::startServer(int port)
{
    if (!this->listen(QHostAddress::Any, port))
    {
        qCritical() << "Could not start server";
    }
    else
    {
        qDebug() << "Server started on port" << port;
    }
}

//Выключение сервера
void ServerLogic::shutdownServer()
{
    this->close();
    Logger::getInstance()->logToFile("Server is turned off");

    //Отключение всех клиентов
    foreach(QTcpSocket *socket, userSockets)
    {
        if (socket->state() == QTcpSocket::ConnectedState)
        {
            socket->disconnectFromHost();
        }
        socket->deleteLater();
    }
    userSockets.clear();

    //Закрыть соединение с базой данных, если открыто
    if (database.isOpen())
    {
        database.close();
    }
}


