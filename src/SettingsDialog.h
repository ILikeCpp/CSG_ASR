#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QJsonObject>

class QLineEdit;
class QSpinBox;
class SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

signals:

private:
    void readConfig();
    bool writeConfig();
    void initLineEdit();

private:
    QJsonObject m_config;
    QLineEdit *m_ip;
    QSpinBox *m_port;
};

#endif // SETTINGSDIALOG_H
