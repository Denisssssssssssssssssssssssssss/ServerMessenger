#include "serverui.h"
#include "logger.h"

#include <QDesktopServices>
#include <QFile>
/**
 * @brief Конструктор класса ServerUI.
 *
 * Инициализирует пользовательский интерфейс и устанавливает необходимые соединения для
 * обработки событий, связанных с логами.
 *
 * @param parent Указатель на родительский виджет (по умолчанию nullptr).
 */
ServerUI::ServerUI(QWidget *parent) : QMainWindow(parent) {
    setupUI();
    connect(logFileButton, &QPushButton::clicked, this, &ServerUI::selectLogFile);
    connect(logUpdateTimer, &QTimer::timeout, this, &ServerUI::updateLogViewer);
    connect(setDefaultLogFileButton, &QPushButton::clicked, this, &ServerUI::makeLogFileDefault);
    connect(logFileNameLabel, &QLabel::linkActivated, this, &ServerUI::openLogFileDirectory);
    updateLogViewer();
}

/**
 * @brief Настройка пользовательского интерфейса.
 *
 * Создает и конфигурирует виджеты пользовательского интерфейса, включая элементы
 * управления для отображения логов и управления ассоциированными файлами.
 */
void ServerUI::setupUI() {
    setWindowIcon(QIcon(":/images/logo.png"));
    resize(window_width, window_height);

    QWidget *centralWidget = new QWidget(this);
    layout = new QVBoxLayout(centralWidget);
    currentLogFilePath = Logger::getInstance()->getDefaultLogPath();

    logFileNameLabel = new QLabel(tr("<a href=\"%1\" style=\"color:#1E90FF;\">Текущий файл логгирования: %2</a>")
                                      .arg(currentLogFilePath)
                                      .arg(QFileInfo(currentLogFilePath).fileName()));
    logFileNameLabel->setAlignment(Qt::AlignRight);

    statusLabel = new QLabel("Сервер работает корректно.");
    statusLabel->setAlignment(Qt::AlignLeft);
    statusLabel->setStyleSheet("QLabel { color : green; }");

    logViewer = new QPlainTextEdit();
    logViewer->setReadOnly(true);
    logFileButton = new QPushButton("Указать файл логгирования");
    setDefaultLogFileButton = new QPushButton("Сделать файлом логгирования по умолчанию");

    headerLayout = new QHBoxLayout();
    headerLayout->addWidget(logFileNameLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(statusLabel);

    layout->addLayout(headerLayout);
    layout->addWidget(logFileButton);
    layout->addWidget(setDefaultLogFileButton);
    layout->addWidget(logViewer);

    setCentralWidget(centralWidget);
    this->setWindowTitle("СЕРВЕР");

    logUpdateTimer = new QTimer(this);
    logUpdateTimer->start(1000);
}

/**
 * @brief Обновляет окно с логами.
 *
 * Читает содержимое текущего файла логов и отображает его в окне логов.
 */
void ServerUI::updateLogViewer() {
    QFile logFile(currentLogFilePath);
    if (logFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&logFile);
        logViewer->setPlainText(stream.readAll());
        logFile.close();
        QScrollBar *scrollBar = logViewer->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
}

/**
 * @brief Открывает диалог выбора файла для логирования.
 *
 * Позволяет пользователю выбрать файл логов и обновляет текущее отображение
 * логов, если выбор был успешным.
 */
void ServerUI::selectLogFile() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Открыть файл"), QDir::homePath(), tr("Log Files (*.txt)"));
    if (!filename.isEmpty()) {
        Logger::getInstance()->setLogFile(filename);
        currentLogFilePath = filename;
        updateLogViewer();
        logFileNameLabel->setText(tr("<a href=\"%1\" style=\"color:#1E90FF;\">Файл логов: %2</a>")
                                      .arg(currentLogFilePath)
                                      .arg(QFileInfo(currentLogFilePath).fileName()));
    }
}

/**
 * @brief Обрабатывает событие закрытия окна.
 *
 * Отображает сообщение с запросом на подтверждение закрытия сервера и
 * посылает соответствующий сигнал при положительном ответе.
 *
 * @param event Указатель на событие закрытия.
 */
void ServerUI::closeEvent(QCloseEvent *event) {
    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "СЕРВЕР",
                                                               tr("Завершить работу сервера?"),
                                                               QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);
    if (resBtn == QMessageBox::Yes) {
        emit serverCloseRequested();  // Испускаем сигнал
        event->accept();
    } else {
        event->ignore();
    }
}

/**
 * @brief Устанавливает текущий файл логов как файл по умолчанию.
 *
 * Запрашивает подтверждение у пользователя и сохраняет выбранный файл
 * как файл логирования по умолчанию, если файл выбран.
 */
void ServerUI::makeLogFileDefault() {
    if(currentLogFilePath.isEmpty()) {
        QMessageBox::information(this, "Информация", "Пожалуйста, сначала выберите файл логов.");
        return;
    }

    QMessageBox::StandardButton resBtn = QMessageBox::question(this, "Подтверждение",
                                                               tr("Сделать текущий файл логов (%1) файлом по умолчанию?").arg(QFileInfo(currentLogFilePath).fileName()),
                                                               QMessageBox::No | QMessageBox::Yes, QMessageBox::Yes);

    if (resBtn == QMessageBox::Yes) {
        Logger::getInstance()->saveDefaultLogPath(currentLogFilePath); // Сохраняет путь к лог-файлу как настройку
        QMessageBox::information(this, "Информация", "Файл по умолчанию обновлен.");
    }
}

/**
 * @brief Открывает директорию, содержащую файл логов.
 *
 * @param link Путь к файлу логов, для которого нужно открыть директорию.
 */
void ServerUI::openLogFileDirectory(const QString &link) {
    QFileInfo fileInfo(link);
    QString folderPath = fileInfo.absolutePath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}
