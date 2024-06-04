#include "serverui.h"
#include "logger.h"

#include <QDesktopServices>
#include <QFile>

ServerUI::ServerUI(QWidget *parent) : QMainWindow(parent)
{
    setupUI();
    connect(logFileButton, &QPushButton::clicked, this, &ServerUI::selectLogFile);
    connect(logUpdateTimer, &QTimer::timeout, this, &ServerUI::updateLogViewer);
    connect(setDefaultLogFileButton, &QPushButton::clicked, this, &ServerUI::makeLogFileDefault);
    connect(logFileNameLabel, &QLabel::linkActivated, this, &ServerUI::openLogFileDirectory);
    updateLogViewer();
}

//Настройка пользовательского интерфейса
void ServerUI::setupUI()
{
    setWindowIcon(QIcon(":/images/logo.png"));
    resize(window_width, window_height);
    QWidget *centralWidget = new QWidget(this); // Создаем центральный виджет
    layout = new QVBoxLayout(centralWidget); // Устанавливаем макет для центрального виджета
    currentLogFilePath = Logger::getInstance()->getDefaultLogPath();
    logFileNameLabel = new QLabel(tr("<a href=\"%1\" style=\"color:#1E90FF;\">Файл логов: %2</a>").arg(currentLogFilePath)
                                      .arg(QFileInfo(currentLogFilePath).fileName()));
    logFileNameLabel->setAlignment(Qt::AlignRight);
    statusLabel = new QLabel("Сервер работает.");
    statusLabel->setAlignment(Qt::AlignLeft);
    statusLabel->setStyleSheet("QLabel { color : green; }");
    logViewer = new QPlainTextEdit();
    logViewer->setReadOnly(true);
    logFileButton = new QPushButton("Выбрать файл для логгирования");
    setDefaultLogFileButton = new QPushButton("Сделать файлом логов по умолчанию");
    headerLayout = new QHBoxLayout();
    headerLayout->addWidget(logFileNameLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(statusLabel);
    layout->addLayout(headerLayout);
    layout->addWidget(logViewer);
    layout->addWidget(logFileButton);
    layout->addWidget(setDefaultLogFileButton);
    setCentralWidget(centralWidget); // Устанавливаем центральный виджет в QMainWindow
    this->setWindowTitle("Сервер");
    logUpdateTimer = new QTimer(this);
    logUpdateTimer = new QTimer(this);
    logUpdateTimer->start(1000);
}

//Обновление окна с логами
void ServerUI::updateLogViewer()
{
    QFile logFile(currentLogFilePath);
    if (logFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&logFile);
        logViewer->setPlainText(stream.readAll());
        logFile.close();
        QScrollBar *scrollBar = logViewer->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
}

//Выбор файла для логгирования
void ServerUI::selectLogFile()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Открыть файл"), QDir::homePath(), tr("Log Files (*.txt)"));
    if(!filename.isEmpty())
    {
        Logger::getInstance()->setLogFile(filename);
        currentLogFilePath = filename;
        updateLogViewer();
        logFileNameLabel->setText(tr("<a href=\"%1\" style=\"color:#1E90FF;\">Файл логов: %2</a>").arg(currentLogFilePath)
            .arg(QFileInfo(currentLogFilePath).fileName()));
    }
}

//При попытке закрыть окно
void ServerUI::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Сервер",
        tr("Вы уверены, что хотите завершить работу сервера?"),
        QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes)
    {
        emit serverCloseRequested();  // Испускаем сигнал
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

//Установка файла логов по умолчанию
void ServerUI::makeLogFileDefault()
{
    if(currentLogFilePath.isEmpty())
    {
        QMessageBox::information(this, "Информация", "Пожалуйста, сначала выберите файл логов.");
        return;
    }

    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Подтверждение",
        tr("Сделать текущий файл логов (%1) файлом по умолчанию?").arg(QFileInfo(currentLogFilePath).fileName()),
        QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);

    if (resBtn == QMessageBox::Yes)
    {
        Logger::getInstance()->saveDefaultLogPath(currentLogFilePath); // Сохраняет путь к лог-файлу как настройку
        QMessageBox::information(this, "Информация", "Файл по умолчанию обновлен.");
    }
}

//Открыть расположение файла логов
void ServerUI::openLogFileDirectory(const QString &link)
{
    QFileInfo fileInfo(link);
    QString folderPath = fileInfo.absolutePath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}
