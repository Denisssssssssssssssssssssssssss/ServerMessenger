#include "Logger.h"

Logger* Logger::instance = nullptr;

Logger::Logger() { loadSettings(); }

//Для получения указателя
Logger* Logger::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Logger();
    }
    return instance;
}

//Запись в файл
void Logger::logToFile(const QString &message)
{
    if (logFile.isOpen())
    {
        QTextStream stream(&logFile);
        QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        stream << timeStamp << " " << message << "\n";
    } else
    {
        qDebug() << "LogFile is not open. Message: " << message;
    }
}

//Выбор файла для записи
void Logger::setLogFile(const QString &filename)
{
    if (logFile.isOpen())
    {
        logFile.close();
    }
    logFile.setFileName(filename);
    if(!logFile.open(QFile::WriteOnly | QFile::Append))
    {
        qDebug() << "Failed to open log file:" << filename;
    }
}

//Загрузка настроек (хранят файл для логов по умолчанию)
void Logger::loadSettings()
{
    QSettings settings(QDir::homePath() + "/appsettings.ini", QSettings::IniFormat);
    QString defaultLogPath = settings.value("Logging/defaultLogPath", QDir::homePath() + "/default_log.txt").toString();
    setLogFile(defaultLogPath);
}

//Сохранить стандартный путь к файлу логов
void Logger::saveDefaultLogPath(const QString &path)
{
    QSettings settings(QDir::homePath() + "/appsettings.ini", QSettings::IniFormat);
    settings.setValue("Logging/defaultLogPath", path);
     settings.sync();
}

//Получить путь к файлу логов по умолчанию
QString Logger::getDefaultLogPath()
{
    QSettings settings(QDir::homePath() + "/appsettings.ini", QSettings::IniFormat);
    return settings.value("Logging/defaultLogPath", QDir::homePath() + "/default_log.txt").toString();
}
