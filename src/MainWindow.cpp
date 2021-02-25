#include "MainWindow.h"
#include "common_define.h"
#include <QApplication>
#include <QDebug>
#include <QPushButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    m_asrService(std::make_shared<SinoVoiceASRService>()),
    m_asrListennerPtr(std::make_shared<GCSClientMgr>())
{
//    qDebug() << "MainWindow currentThread:" << QThread::currentThread();

    setFixedSize(450,100);
    initTitle();
    initBtn();
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
    title += "_";
    title += csg_getCompileDatetime();
    title += "_";
    title += csg_debug_release();
    setWindowTitle(title.c_str());
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
