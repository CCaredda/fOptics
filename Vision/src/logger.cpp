#include "logger.h"

QString Logger::logFilePath = QCoreApplication::applicationDirPath() + "/default_log.txt";
QMutex Logger::mutex;

Logger::Logger(QObject *parent)
    : QObject{parent}
{
    logFilePath ="";
}


Logger::~Logger()
{

}

void Logger::install()
{
    qInstallMessageHandler(Logger::messageHandler);
}

void Logger::setLogFile(const QString &newPath)
{
    QMutexLocker locker(&mutex);
    logFilePath = newPath;
}

QString Logger::currentLogFile()
{
    QMutexLocker locker(&mutex);
    return logFilePath;
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&mutex);

    QString level;
    switch (type) {
    case QtDebugMsg:    level = "DEBUG"; break;
    case QtInfoMsg:     level = "INFO"; break;
    case QtWarningMsg:  level = "WARNING"; break;
    case QtCriticalMsg: level = "CRITICAL"; break;
    case QtFatalMsg:    level = "FATAL"; break;
    }

    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString line = QString("[%1] [%2] %3 (%4:%5, %6)")
                       .arg(time)
                       .arg(level)
                       .arg(msg)
                       .arg(context.file ? context.file : "")
                       .arg(context.line)
                       .arg(context.function ? context.function : "");

    QFile file(logFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream ts(&file);
        ts << line << Qt::endl;
    }

    if (type == QtFatalMsg)
        abort();
}
