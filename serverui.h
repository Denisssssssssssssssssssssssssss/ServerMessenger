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

class ServerUI : public QMainWindow
{
    Q_OBJECT

private:
    QLabel* statusLabel;
    QPushButton* logFileButton;
    QVBoxLayout* layout;
    unsigned int window_width = 450;
    unsigned int window_height = 300;
    QPlainTextEdit* logViewer;
    QTimer* logUpdateTimer;
    QString currentLogFilePath;
    QLabel* logFileNameLabel;
    QHBoxLayout *headerLayout;
    QPushButton* setDefaultLogFileButton;

    void setupUI();
    void updateLogViewer();
    void selectLogFile();
    void makeLogFileDefault();

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    ServerUI(QWidget *parent = nullptr);

signals:
    void serverCloseRequested();

private slots:
    void openLogFileDirectory(const QString &link);
};
#endif // SERVERUI_H
