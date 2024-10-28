#include "Logger.h"

Logger* Logger::instance = nullptr; ///< Указатель на единственный экземпляр Logger.

/**
 * /brief Конструктор класса Logger.
 *
 * Инициализирует экземпляр логгера и загружает настройки при создании объекта.
 */
Logger::Logger() {
    loadSettings();
}

/**
 * /brief Получает указатель на единственный экземпляр классов Logger.
 *
 * Если экземпляр еще не создан, он будет инициализирован.
 *
 * /return Указатель на экземпляр Logger.
 */
Logger* Logger::getInstance()
{
    if (instance == nullptr)
    {
        instance = new Logger(); // Создание нового экземпляра, если он не существует.
    }
    return instance;
}

/**
 * /brief Записывает сообщение в лог-файл.
 *
 * Если файл журнала открыт, в него записывается текущее время и сообщение.
 * В противном случае выводится предупреждение в отладочный вывод.
 *
 * /param message Сообщение, которое нужно записать в лог.
 */
void Logger::logToFile(const QString &message)
{
    if (logFile.isOpen())
    {
        QTextStream stream(&logFile);
        QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        stream << timeStamp << " " << message << "\n"; // Запись временной метки и сообщения.
    }
    else
    {
        qDebug() << "LogFile is not open. Message: " << message; // Вывод сообщения об ошибке, если файл не открыт.
    }
}

/**
 * /brief Устанавливает файл для записи логов.
 *
 * Закрывает текущий файл, если он открыт, и открывает новый файл
 * для записи логов. Если открытие не удается, выводит сообщение об ошибке.
 *
 * /param filename Имя файла для логирования.
 */
void Logger::setLogFile(const QString &filename)
{
    if (logFile.isOpen())
    {
        logFile.close(); // Закрытие текущего файла журнала.
    }
    logFile.setFileName(filename); // Установка нового файла для записи.
    if(!logFile.open(QFile::WriteOnly | QFile::Append))
    {
        qDebug() << "Failed to open log file:" << filename; // Сообщение об ошибке, если файл открыть не удалось.
    }
}

/**
 * /brief Загружает настройки логирования из файла конфигурации.
 *
 * Читает путь к файлу журнала по умолчанию из файла appsettings.ini
 * и устанавливает его как текущий файл для записи логов.
 */
void Logger::loadSettings()
{
    QSettings settings(QDir::homePath() + "/appsettings.ini", QSettings::IniFormat);
    QString defaultLogPath = settings.value("Logging/defaultLogPath", QDir::homePath() + "/default_log.txt").toString();
    setLogFile(defaultLogPath); // Устанавливает файл для логирования по умолчанию.
}

/**
 * /brief Сохраняет стандартный путь к файлу логов в настройки.
 *
 * /param path Путь, который нужно сохранить как стандартный путь к файлу логов.
 */
void Logger::saveDefaultLogPath(const QString &path)
{
    QSettings settings(QDir::homePath() + "/appsettings.ini", QSettings::IniFormat);
    settings.setValue("Logging/defaultLogPath", path); // Сохраняет новый путь в настройки.
    settings.sync(); // Синхронизирует настройки с файлом.
}

/**
 * /brief Получает путь к файлу логов по умолчанию из настроек.
 *
 * /return Путь к файлу журнала, хранящемуся по умолчанию.
 */
QString Logger::getDefaultLogPath()
{
    QSettings settings(QDir::homePath() + "/appsettings.ini", QSettings::IniFormat);
    return settings.value("Logging/defaultLogPath", QDir::homePath() + "/default_log.txt").toString(); // Возвращает путь к файлу журнала по умолчанию.
}
