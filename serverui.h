/**
 * /file serverui.h
 * /brief Определение интерфейса сервера с помощью графического интерфейса пользователя.
 */

#ifndef SERVERUI_H
#define SERVERUI_H

#include <QMainWindow>
#include <QWidget>
#include <QTextEdit>
#include <QScrollBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

/**
 * /brief Класс ServerUI.
 *
 * Класс предоставляет графический интерфейс для сервера, включая отображение логов
 * и настройки конфигурации файла журнала. Наследует от QMainWindow.
 */
class ServerUI : public QMainWindow
{
    Q_OBJECT

private:
    QLabel* statusLabel; ///< Метка для отображения статуса сервера.
    QPushButton* logFileButton; ///< Кнопка для выбора файла журнала.
    QVBoxLayout* layout; ///< Основной вертикальный макет для размещения элементов интерфейса.
    unsigned int window_width = 450; ///< Ширина окна.
    unsigned int window_height = 300; ///< Высота окна.
    QPlainTextEdit* logViewer; ///< Поле для просмотра содержимого журнала.
    QTimer* logUpdateTimer; ///< Таймер для обновления содержимого журнала.
    QString currentLogFilePath; ///< Путь к текущему файлу журнала.
    QLabel* logFileNameLabel; ///< Метка для отображения имени файла журнала.
    QHBoxLayout *headerLayout; ///< Горизонтальный макет для заголовка.
    QPushButton* setDefaultLogFileButton; ///< Кнопка для установки файла журнала по умолчанию.

    /**
     * /brief Настройка пользовательского интерфейса.
     *
     * Инициализирует все элементы интерфейса и устанавливает их свойства.
     */
    void setupUI();

    /**
     * /brief Обновляет содержимое поля просмотра журнала.
     *
     * Читает данные из текущего файла журнала и обновляет отображение.
     */
    void updateLogViewer();

    /**
     * /brief Открывает диалог для выбора файла журнала.
     *
     * Позволяет пользователю выбрать файл и обновляет путь к файлу журнала.
     */
    void selectLogFile();

    /**
     * /brief Устанавливает выбранный файл журнала как файл по умолчанию.
     */
    void makeLogFileDefault();

private slots:
    /**
     * /brief Обрабатывает запрос на открытие директории файла журнала.
     *
     * /param link Ссылка на директорию файла журнала.
     */
    void openLogFileDirectory(const QString &link);

protected:
    /**
     * /brief Обрабатывает событие закрытия окна.
     *
     * /param event Указатель на объект события закрытия.
     */
    void closeEvent(QCloseEvent *event) override;

public:
    /**
     * /brief Конструктор класса ServerUI.
     *
     * Инициализирует интерфейс пользователя и устанавливает родительский объект.
     *
     * /param parent Указатель на родительский объект.
     */
    ServerUI(QWidget *parent = nullptr);

signals:
    /**
     * /brief Сигнал, который отправляется при запросе закрытия сервера.
     */
    void serverCloseRequested();
};

#endif // SERVERUI_H
