#include "MainWindow.h"
#include "common_define.h"
#include <QApplication>
#include <QDebug>
#include <QPushButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include "SettingsDialog.h"

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    m_asrService(std::make_shared<SinoVoiceASRService>()),
    m_asrListennerPtr(std::make_shared<GCSClientMgr>())
{
//    qDebug() << "MainWindow currentThread:" << QThread::currentThread();

    resize(300,150);
    initTitle();
    initMenu();
    initBtn();
    initQSS();
    m_asrService->attachASRListenner(m_asrListennerPtr);
}

MainWindow::~MainWindow()
{
    m_asrService->detachASRListenner(m_asrListennerPtr);
    m_asrService->stopService();
}

void MainWindow::initTitle()
{
    std::string title = QApplication::applicationDisplayName().toStdString();
    title += "_";
    title += csg_getSoftwareVersion();
//    title += "_";
//    title += csg_getCompileDatetime();
//    title += "_";
//    title += csg_debug_release();
    setWindowTitle(title.c_str());
}

void MainWindow::initMenu()
{
    QMenu *settingMenu = this->menuBar()->addMenu(tr("设置"));
    QMenu *helpMenu = this->menuBar()->addMenu(tr("帮助"));

    settingMenu->addAction(QIcon(":/img/image/setting.png"), tr("设置服务器"),[=]{
        SettingsDialog dialog(this);
        dialog.setWindowFlag(Qt::WindowContextHelpButtonHint,false);
        dialog.exec();
    });

    helpMenu->addAction(QIcon(":/img/image/help.png"), tr("版本信息"),[=]{
        QMessageBox::information(this,tr("版本信息"),tr("版本号: %1\n编译时间: %2\nDebug/Release: %3")
                                 .arg(QString::fromStdString(csg_getSoftwareVersion()))
                                 .arg(QString::fromStdString(csg_getCompileDatetime()))
                                 .arg(QString::fromStdString(csg_debug_release())));
    });

    helpMenu->addAction(QIcon(":/img/image/about.png"), tr("关于"),[=]{
        QMessageBox::aboutQt(this, tr("关于Qt"));
    });
}

void MainWindow::initBtn()
{
    QFrame *frame = new QFrame(this);
    QHBoxLayout *mainLayout = new QHBoxLayout();
    frame->setLayout(mainLayout);

    mainLayout->addStretch();
    QPushButton *startBtn = new QPushButton("启动服务",frame);
    startBtn->setEnabled(true);
    mainLayout->addWidget(startBtn);
    mainLayout->addStretch();
    QPushButton *stopBtn = new QPushButton("停止服务",frame);
    stopBtn->setEnabled(false);
    mainLayout->addWidget(stopBtn);
    mainLayout->addStretch();

    setCentralWidget(frame);

    connect(startBtn, &QPushButton::clicked, [=]{
        startBtn->setEnabled(false);
        stopBtn->setEnabled(true);
        m_asrService->startService();
    });

    connect(stopBtn, &QPushButton::clicked, [=]{
        startBtn->setEnabled(true);
        stopBtn->setEnabled(false);
        m_asrService->stopService();
    });
}

void MainWindow::initQSS()
{
    this->setStyleSheet("QFrame,QDialog {"
                        "background:#EDFEFE"
                        "}"
                        "QMenuBar {"
                        "background:#EDFEFE"
                        "}"
                        "QPushButton"
                        "{"
                        "border-width:1px;"
                        "border-style:solid;"
                        "border-radius:5px;"
                        "border-color:#9BC2E6;"
                        "min-width:80px;"
                        "min-height:30px;"
                        "}"
                        "QPushButton:!enabled {"
                        "border-color:gray"
                        "}"
                        "QPushButton:pressed {"
                        "background:#9BC2E6;"
                        "}"
                        "QPushButton:hover:!pressed {"
                        "background-color:#90EE90;"
                        "}"
                        );
}
