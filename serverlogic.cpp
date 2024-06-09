#include "serverlogic.h"

#include <QSqlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QJsonArray>
#include <QCryptographicHash>


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
        // Прием данных от клиента
        QByteArray jsonData = clientSocket->readAll();
        QJsonParseError parseError;
        QJsonDocument document = QJsonDocument::fromJson(jsonData, &parseError);

        if (parseError.error != QJsonParseError::NoError)
        {
            // Отправить ответ клиенту о неверном формате JSON
            clientSocket->write("{\"status\":\"error\",\"message\":\"Invalid JSON format\"}");
            return;
        }

        QJsonObject json = document.object();
        qDebug() << "Received JSON:" << json;

        // Обработка запроса на регистрацию
        if (json.contains("type") && json["type"].toString() == "register" &&
            json.contains("login") && json.contains("password"))
        {
            QString login = json["login"].toString();
            QString hashedPassword = json["password"].toString();

            // Проверка допустимости логина
            if (loginAvailable(login) && loginContainsOnlyAllowedCharacters(login))
            {

                // Добавление пользователя в базу данных
                QSqlQuery query(database);
                query.prepare("INSERT INTO user_auth (login, password, nickname) "
                              "VALUES (:login, :password, :nickname)");
                query.bindValue(":login", login);
                query.bindValue(":password", hashedPassword); // Сохраняем полученный от клиента хеш пароля
                query.bindValue(":nickname", "New user"); // Используем логин в качестве никнейма
                if (!query.exec())
                {
                    // Ошибка при добавлении пользователя в БД
                    clientSocket->write("{\"status\":\"error\",\"message\":\"Failed to register user\"}");
                }
                else
                {
                    // Пользователь успешно добавлен в БД
                    clientSocket->write("{\"status\":\"success\",\"message\":\"User registered successfully\"}");
                    Logger::getInstance()->logToFile(QString("User '%1' was successfully registered.").arg(login));
                }
            }
            else
            {
                // Информировать клиента о недопустимости логина
                clientSocket->write("{\"status\":\"error\",\"message\":\"Login validation failed\"}");
            }
        }
        else if(json.contains("type") && json["type"].toString() == "register")
        {
            clientSocket->write("{\"status\":\"error\",\"message\":\"Missing required fields\"}");
        }

        else if (json.contains("type") && json["type"].toString() == "login" &&
            json.contains("login") && json.contains("password"))
        {
            QString login = json["login"].toString();
            QString hashedPassword = json["password"].toString();

            QSqlQuery query(database);
            query.prepare("SELECT password FROM user_auth WHERE login = :login");
            query.bindValue(":login", login);
            query.exec();

            if (query.next())
            {
                QString storedPassword = query.value(0).toString();
                if(storedPassword == hashedPassword)
                {
                    // Пароли совпадают, успешный вход
                    clientSocket->write("{\"status\":\"success\",\"message\":\"Logged in successfully\"}");
                    Logger::getInstance()->logToFile(QString("User '%1' logged in successfully.").arg(login));
                }
                else
                {
                    // Пароли не совпадают
                    clientSocket->write("{\"status\":\"error\",\"message\":\"Login failed. Incorrect password.\"}");
                }
            }
            else if(json.contains("type") && json["type"].toString() == "login") //???
            {
                // Логин не найден в базе данных
                clientSocket->write("{\"status\":\"error\",\"message\":\"Login failed. User not found.\"}");
            }
        }

        else if (json.contains("type") && json["type"].toString() == "check_nickname" && json.contains("login"))
        {
            QString login = json["login"].toString();
            QSqlQuery query(database);
            query.prepare("SELECT nickname FROM user_auth WHERE login = :login");
            query.bindValue(":login", login);
            if(query.exec() && query.next()) {
                QString nickname = query.value(0).toString();
                QJsonObject response;
                response["type"] = "check_nickname";
                response["status"] = "success";
                response["nickname"] = nickname;
                qDebug() << nickname << "\n";
                // Отправить найденный никнейм обратно клиенту
                qDebug() << QJsonDocument(response).toJson(QJsonDocument::Compact);
                clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact));
                clientSocket->flush();
            }
        }
        else if (json.contains("type") && json["type"].toString() == "update_nickname" &&
                 json.contains("login") && json.contains("nickname")) {
            QString login = json["login"].toString();
            QString nickname = json["nickname"].toString();

            // Проверка никнейма на допустимость
            if (!nickname.isEmpty() && nickname != "New user") {
                QSqlQuery query(database);
                query.prepare("UPDATE user_auth SET nickname = :nickname WHERE login = :login");
                query.bindValue(":nickname", nickname);
                query.bindValue(":login", login);
                if (!query.exec()) {
                    QJsonObject response;
                    response["type"] = "update_nickname";
                    response["status"] = "error";
                    response["message"] = "Не удалось обновить имя.";
                    clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact));
                } else {
                    QJsonObject response;
                    response["type"] = "update_nickname";
                    response["status"] = "success";
                    response["message"] = "Nickname has been changed.";
                    QString logMessage = QString("User with login '%1' has changed their name to '%2'").arg(login, nickname);
                    Logger::getInstance()->logToFile(logMessage);
                    clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact));
                }
            } else {
                QJsonObject response;
                response["type"] = "update_nickname";
                response["status"] = "error";
                response["message"] = "Недопустимое имя.";
                clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact));
            }
            clientSocket->flush();
        }
        else if (json.contains("type") && json["type"].toString() == "find_users" && json.contains("searchText") && json.contains("login")) {
                QString searchText = json["searchText"].toString();
                QString userLogin = json["login"].toString(); // Получаем логин пользователя, отправившего запрос
                QSqlQuery query(database);
                // Измените SQL запрос, добавив условие для исключения логина пользователя из результатов
                query.prepare("SELECT * FROM user_auth WHERE nickname LIKE :nickname AND login != :login");
                query.bindValue(":nickname", '%' + searchText + '%');
                query.bindValue(":login", userLogin); // Исключаем пользователя из результатов
                if (!query.exec()) {
                    QJsonObject response;
                    response["status"] = "error";
                    response["message"] = "Ошибка при поиске пользователей.";
                    clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact));
                } else {
                    QJsonArray usersArray;
                    while (query.next()) {
                        QString nickname = query.value("nickname").toString();
                        usersArray.append(QJsonValue(nickname));
                    }
                    QJsonObject response;
                    response["status"] = "success";
                    response["users"] = usersArray;
                    clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact));
                }
                clientSocket->flush();
        }
        else if (json.contains("type") && json["type"].toString() == "update_login" &&
                 json.contains("old_login") && json.contains("new_login") && json.contains("password"))
        {
            QString oldLogin = json["old_login"].toString();
            QString newLogin = json["new_login"].toString();
            QString clientPassword = json["password"].toString();

            // Проверка допустимости логина и нового логина
            if (!loginAvailable(newLogin) || !loginContainsOnlyAllowedCharacters(newLogin)) {
                clientSocket->write("{\"type\":\"update_login\", \"status\":\"error\",\"message\":\"Invalid or duplicate new login.\"}");
                clientSocket->flush();
                return;
            }

            QSqlQuery query(database);

            // Проверяем существование старого логина и его пароля
            query.prepare("SELECT password FROM user_auth WHERE login = :oldLogin");
            query.bindValue(":oldLogin", oldLogin);
            if (query.exec() && query.next()) {
                QString dbHashedPassword = query.value(0).toString();

                // Если пароли совпадают
                if (getSha256Hash(clientPassword, oldLogin) == dbHashedPassword) {
                    // Зашифровываем пароль с использованием нового логина как соли
                    QString newHashedPassword = getSha256Hash(clientPassword, newLogin); // Функция getSha256Hash() должна быть реализована вами

                    // Обновляем данные пользователя в БД
                    query.prepare("UPDATE user_auth SET login = :newLogin, password = :newHashedPassword WHERE login = :oldLogin");
                    query.bindValue(":newLogin", newLogin);
                    query.bindValue(":newHashedPassword", newHashedPassword);
                    query.bindValue(":oldLogin", oldLogin);

                    if (query.exec()) {
                        clientSocket->write("{\"type\":\"update_login\", \"status\":\"success\",\"message\":\"Login and password updated successfully.\"}");
                    } else {
                        clientSocket->write("{\"type\":\"update_login\", \"status\":\"error\",\"message\":\"Could not update login and password in the database.\"}");
                    }
                } else {
                    clientSocket->write("{\"type\":\"update_login\", \"status\":\"error\",\"message\":\"Incorrect old password.\"}");
                }
            } else {
                clientSocket->write("{\"type\":\"update_login\", \"status\":\"error\",\"message\":\"Old login not found.\"}");
            }
            clientSocket->flush();
        }
        else if (json.contains("type") && json["type"].toString() == "update_password" &&
                 json.contains("login") && json.contains("current_password") && json.contains("new_password"))
        {
            QString login = json["login"].toString();
            QString currentPassword = json["current_password"].toString(); // предполагается, что пароль хэшируется на клиенте
            QString newPassword = json["new_password"].toString(); // предполагается, что пароль хэшируется на клиенте

            QSqlQuery query(database);
            query.prepare("SELECT password FROM user_auth WHERE login = :login");
            query.bindValue(":login", login);
            if (query.exec() && query.next()) {
                QString storedPassword = query.value(0).toString();

                if (storedPassword == currentPassword) {

                    query.prepare("UPDATE user_auth SET password = :newPassword WHERE login = :login");
                    query.bindValue(":newPassword", newPassword);
                    query.bindValue(":login", login);

                    if (query.exec()) {
                        clientSocket->write("{\"type\":\"update_password\", \"status\":\"success\",\"message\":\"Password updated successfully.\"}");
                    } else {
                        clientSocket->write("{\"type\":\"update_password\", \"status\":\"error\",\"message\":\"Could not update password.\"}");
                    }
                } else {
                    clientSocket->write("{\"type\":\"update_password\", \"status\":\"error\",\"message\":\"Incorrect current password.\"}");
                }
            } else {
                clientSocket->write("{\"type\":\"update_password\", \"status\":\"error\",\"message\":\"Login not found.\"}");
            }
            clientSocket->flush();
        }

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

bool ServerLogic::passwordContainsRequiredCharacters(const QString &password)
{
    QRegularExpression upperCaseRegExp("[A-Z]"); //Регулярное выражение для заглавных букв
    QRegularExpression lowerCaseRegExp("[a-z]"); //Регулярное выражение для строчных букв
    QRegularExpression digitRegExp("\\d");       //Регулярное выражение для цифр
    QRegularExpression specialRegExp("[!@#$%^&*()_+=-]"); //Регулярное выражение для специальных символов

    return password.contains(upperCaseRegExp) &&
           password.contains(lowerCaseRegExp) &&
           password.contains(digitRegExp) &&
           password.contains(specialRegExp);
}

bool ServerLogic::loginContainsOnlyAllowedCharacters(const QString &login)
{
    QRegularExpression loginRegExp("^[A-Za-z\\d_-]+$"); //Регулярное выражение для допустимых символов в логине
    return login.contains(loginRegExp);
}

//Проверка доступности логина при регистрации
bool ServerLogic::loginAvailable(const QString& login)
{
    QSqlQuery query(database);
    query.prepare("SELECT COUNT(*) FROM user_auth WHERE login = :login");
    query.bindValue(":login", login);
    query.exec();

    if (query.next() && query.value(0).toInt() == 0) {
        return true; //Логин свободен
    }
    return false; //логин занят
}

QString ServerLogic::getSha256Hash(const QString &str, const QString &salt) {
    QByteArray byteArrayPasswordSalt = (str + salt).toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(byteArrayPasswordSalt, QCryptographicHash::Sha256).toHex();
    return hashedPassword;
}
