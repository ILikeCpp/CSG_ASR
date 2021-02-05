#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "SinoVoiceASRService.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    void initTitle();
    void initBtn();

private:
    ASRServicePtr m_asrService;
};

#endif // MAINWINDOW_H
