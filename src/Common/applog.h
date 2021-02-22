#ifndef APPLOG_H
#define APPLOG_H

#include <iostream>
#include <QDate>
#include <QDir>
#include <fstream>
#include <QCoreApplication>

class MyMessage
{
public:
    MyMessage() {

    }

    virtual ~MyMessage() {

    }

    static void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    static std::ofstream fLogFile;
    static bool openLogFile(QString &rLogFilePathP);
};

std::ofstream MyMessage::fLogFile;

void MyMessage::messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString currentTime = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss] ");
    QString currentDate = currentTime.mid(1, 10);
    QString fileName = QString(context.file).section('\\', -1);
    QByteArray localMsg = msg.toLocal8Bit();
    static QString lastDate = currentDate;
    QString level("");

    switch (type) {
    case QtDebugMsg:
        level = "Debug";
        break;
    case QtWarningMsg:
        level = "Warning";
        break;
    case QtCriticalMsg:
        level = "Critical";
        break;
    case QtFatalMsg:
        level = "Fatal";
        break;
    default:
        std::cout<<"unknown message type:"<<type<<std::endl;
        break;
    }

    QString message = currentTime  + "(" + level + ":" + fileName + ":" + QString::number(context.line, 10) + ") " + msg;//localMsg.constData();
    std::string strMessage(message.toLocal8Bit());
    std::cerr<<strMessage<<std::endl;

    QString appPath = QCoreApplication::applicationDirPath();
    appPath = QDir::toNativeSeparators(appPath);
    QString logPath = appPath  + "\\log\\AppLog_" + currentDate + ".txt";
    logPath = QDir::toNativeSeparators(logPath);
    if (false == fLogFile.is_open())
    {
        if (false == openLogFile(logPath))
        {
            return;
        }
    }
    else if (currentDate != lastDate)
    {
        if (false == openLogFile(logPath))
        {
            return;
        }
        lastDate = currentDate;
    }

    fLogFile<<strMessage<<std::endl;
}

bool MyMessage::openLogFile(QString &rLogFilePathP)
{
    fLogFile.close();
    fLogFile.clear();
    fLogFile.open(rLogFilePathP.toStdString().c_str(), std::ios_base::app);
    if(false == fLogFile.is_open())
    {
       std::cout<<"open log file error, file path:"<<rLogFilePathP.toStdString()<<std::endl;
       return false;
    }

    return true;
}


#endif // APPLOG_H
