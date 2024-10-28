/**
 * /file logger.h
 * /brief Определение класса Logger для ведения логов в файле.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <QDebug>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QSettings>

/**
 * /brief Класс Logger.
 *
 * Класс предоставляет функциональность для ведения логов в файл.
 * Реализует шаблон Singleton, позволяя иметь единственный экземпляр логгера в приложении.
 */
class Logger
{
private:
    static Logger* instance; ///< Указатель на единственный экземпляр класса Logger.
    QFile logFile; ///< Файл для записи логов.
    Logger(); ///< Конструктор класса Logger, приватный для предотвращения создания дополнительных экземпляров.

public:
    /**
     * /brief Получает единственный экземпляр класса Logger.
     * /return Указатель на экземпляр Logger.
     */
    static Logger* getInstance();

    /**
     * /brief Записывает сообщение в лог-файл.
     * /param message Сообщение для записи в лог.
     */
    void logToFile(const QString &message);

    /**
     * /brief Устанавливает файл журнала по указанному имени.
     * /param filename Имя файла для журнала.
     */
    void setLogFile(const QString &filename);

    /**
     * /brief Загружает настройки логирования.
     *
     * Читает настройки, включая путь к файлу журнала, из конфигурационного файла.
     */
    void loadSettings();

    /**
     * /brief Сохраняет путь по умолчанию для журнала логов.
     * /param path Путь, который нужно сохранить как путь по умолчанию.
     */
    void saveDefaultLogPath(const QString &path);

    /**
     * /brief Получает путь по умолчанию для журнала логов.
     * /return Строка с путем к файлу журнала.
     */
    QString getDefaultLogPath();
};

#endif // LOGGER_H
