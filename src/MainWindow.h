#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "SinoVoiceASRService.h"
#include "GCSClientMgr.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void initTitle();
    void initMenu();
    void initBtn();
    void initQSS();

private:
    ASRServicePtr m_asrService;
    ASRListennerPtr m_asrListennerPtr;
};

#endif // MAINWINDOW_H
