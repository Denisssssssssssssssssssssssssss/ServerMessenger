#include "serverlogic.h"

#include <QSqlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QCryptographicHash>
#include <QRegularExpression>


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
void ServerLogic::onNewConnection() {
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
                response["nickname"] = nickname;
                qDebug() << nickname << "\n";
                // Отправить найденный никнейм обратно клиенту
                clientSocket->write(QJsonDocument(response).toJson(QJsonDocument::Compact));
                clientSocket->flush();
            }
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
