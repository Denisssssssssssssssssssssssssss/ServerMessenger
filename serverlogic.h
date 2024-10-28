/**
 * /file serverlogic.h
 * /brief Определение класса ServerLogic для работы с логикой сервера.
 */

#ifndef SERVERLOGIC_H
#define SERVERLOGIC_H

#include "logger.h"
#include <QTcpServer>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTcpSocket>
#include <qrsaencryption.h>

/**
 * /brief Класс ServerLogic.
 *
 * Класс реализует логику сервера, наследуя от QTcpServer. Он обрабатывает соединения клиентов,
 * управляет чатами и пользователями, а также выполняет операции с базой данных.
 */
class ServerLogic : public QTcpServer
{
    Q_OBJECT

private:
    QHash<int, QTcpSocket*> userSockets; ///< Хранит сокеты пользователей, связанных с их идентификаторами.
    QSqlDatabase database; ///< Объект базы данных для взаимодействия с SQL-сервером.

    /**
     * /brief Проверяет, содержит ли пароль необходимые символы.
     * /param password Пароль для проверки.
     * /return Признак, соответствует ли пароль требованиям (true, если соответствует).
     */
    bool passwordContainsRequiredCharacters(const QString &password);

    /**
     * /brief Проверяет, содержит ли логин только допустимые символы.
     * /param login Логин для проверки.
     * /return Признак, содержит ли логин только разрешённые символы (true, если содержит).
     */
    bool loginContainsOnlyAllowedCharacters(const QString &login);

    /**
     * /brief Проверяет доступность логина для регистрации.
     * /param login Логин для проверки.
     * /return Признак, доступен ли логин (true, если доступен).
     */
    bool loginAvailable(const QString& login);

    /**
     * /brief Получает SHA-512 хеш строки с солью.
     * /param str Строка для хеширования.
     * /param salt Соль для хеширования.
     * /return Хешированная строка.
     */
    QString getSha512Hash(const QString &str, const QString &salt);

    /**
     * /brief Обрабатывает запрос на получение списка чатов.
     * /param clientSocket Указатель на сокет клиента.
     * /param json Объект JSON с данными запроса.
     */
    void handleGetChatList(QTcpSocket* clientSocket, const QJsonObject &json);

    /**
     * /brief Обрабатывает запрос на отправку сообщения.
     * /param clientSocket Указатель на сокет клиента.
     * /param json Объект JSON с данными запроса.
     */
    void handleSendMessage(QTcpSocket* clientSocket, const QJsonObject &json);

    /**
     * /brief Обрабатывает запрос на получение истории переписки.
     * /param clientSocket Указатель на сокет клиента.
     * /param json Объект JSON с данными запроса.
     */
    void handleGetChatHistory(QTcpSocket* clientSocket, const QJsonObject &json);

    /**
     * /brief Обрабатывает запрос на получение или создание чата.
     * /param clientSocket Указатель на сокет клиента.
     * /param json Объект JSON с данными запроса.
     */
    void handleGetOrCreateChat(QTcpSocket* clientSocket, const QJsonObject &json);

    /**
     * /brief Помечает сообщения как прочитанные.
     * /param chatId Идентификатор чата.
     * /param userId Идентификатор пользователя.
     */
    void markMessagesAsRead(int chatId, int userId);

    /**
     * /brief Обрабатывает запрос на удаление чата.
     * /param clientSocket Указатель на сокет клиента.
     * /param json Объект JSON с данными запроса.
     */
    void handleDeleteChat(QTcpSocket* clientSocket, const QJsonObject &json);

    /**
     * /brief Генерирует RSA ключи для шифрования.
     */
    void generateRSAKeys();

private slots:
    /**
     * /brief Обрабатывает новое подключение клиента к серверу.
     */
    void onNewConnection();

    /**
     * /brief Обрабатывает запрос на создание чата.
     * /param clientSocket Указатель на сокет клиента.
     * /param json Объект JSON с данными запроса.
     */
    void handleCreateChat(QTcpSocket* clientSocket, const QJsonObject &json);

    /**
     * /brief Обрабатывает запрос на поиск пользователей.
     * /param clientSocket Указатель на сокет клиента.
     * /param json Объект JSON с данными запроса.
     */
    void handleFindUsers(QTcpSocket* clientSocket, const QJsonObject &json);

public:
    /**
     * /brief Конструктор класса ServerLogic.
     * /param parent Указатель на родительский объект.
     */
    ServerLogic(QObject *parent = nullptr);

    /**
     * /brief Запускает сервер на указанном порту.
     * /param port Порт, на котором будет слушать сервер.
     */
    void startServer(int port);

public slots:
    /**
     * /brief Останавливает сервер.
     */
    void shutdownServer();
};

#endif // SERVERLOGIC_H
