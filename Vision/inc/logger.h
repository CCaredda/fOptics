#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QString>
#include <QDateTime>
#include <QCoreApplication>

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();
    static void install();
    static void setLogFile(const QString& newPath);
    static QString currentLogFile();

private:
    static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    static QString logFilePath;
    static QMutex mutex;

signals:

};

#endif // LOGGER_H
