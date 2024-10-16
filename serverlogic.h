#ifndef SERVERLOGIC_H
#define SERVERLOGIC_H
#include "logger.h"
#include <QTcpServer>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTcpSocket>
#include <qrsaencryption.h>

class ServerLogic : public QTcpServer
{
    Q_OBJECT

private:
    QHash<int, QTcpSocket*> userSockets;
    QSqlDatabase database;

    bool passwordContainsRequiredCharacters(const QString &password);
    bool loginContainsOnlyAllowedCharacters(const QString &login);
    bool loginAvailable(const QString& login);
    QString getSha512Hash(const QString &str, const QString &salt);
    void handleGetChatList(QTcpSocket* clientSocket, const QJsonObject &json);
    void handleSendMessage(QTcpSocket* clientSocket, const QJsonObject &json);
    void handleGetChatHistory(QTcpSocket* clientSocket, const QJsonObject &json);
    void handleGetOrCreateChat(QTcpSocket* clientSocket, const QJsonObject &json);
    void markMessagesAsRead(int chatId, int userId);
    void handleDeleteChat(QTcpSocket* clientSocket, const QJsonObject &json);
    void generateRSAKeys();


private slots:
    void onNewConnection();
    void handleCreateChat(QTcpSocket* clientSocket, const QJsonObject &json);
    void handleFindUsers(QTcpSocket* clientSocket, const QJsonObject &json);

public:
    ServerLogic(QObject *parent = nullptr);
    void startServer(int port);

public slots:
    void shutdownServer();

};

#endif // SERVERLOGIC_H
