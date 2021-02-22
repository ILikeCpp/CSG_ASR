#include "MainWindow.h"
#include <QApplication>
#include <QDir>
#include "applog.h"

void initAppLog()
{
    QString appPath = QCoreApplication::applicationDirPath();
    appPath = QDir::toNativeSeparators(appPath);
    QString logPath = appPath + "\\log";
    QDir logDir;
    if (!logDir.exists(logPath))
    {
        logDir.mkpath(logPath);
    }
    qInstallMessageHandler(MyMessage::messageOutput);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    initAppLog();

    MainWindow w;
    w.show();

    return a.exec();
}
