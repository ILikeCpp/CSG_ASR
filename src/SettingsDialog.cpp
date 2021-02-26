#include "SettingsDialog.h"
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QTextStream>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include "common_define.h"

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    readConfig();
    initLineEdit();

    connect(this, &SettingsDialog::accepted,[=]{
        this->m_config.insert(CONFIG_JSON_HTTP_SERVER_IP,m_ip->text());
        this->m_config.insert(CONFIG_JSON_HTTP_SERVER_PORT,QString::number(m_port->value()));
        if(!writeConfig())
        {
            QMessageBox::warning(this,tr("警告"),tr("保存失败!"));
        }
    });
}

SettingsDialog::~SettingsDialog()
{
    qDebug() << "SettingsDialog::~SettingsDialog";
}

void SettingsDialog::readConfig()
{
    QFile file("../../data/config/config.json");
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        qDebug() << "file open failed! " << file.errorString();
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(),&err);
    if (err.error != QJsonParseError::NoError)
    {
        qDebug() << err.errorString();
    }
    m_config = doc.object();
    file.close();

//    qDebug() << m_config;
}

bool SettingsDialog::writeConfig()
{
    QFile file("../../data/config/config.json");
    if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        qDebug() << "file open failed! " << file.errorString();
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    QJsonDocument doc(m_config);
    out << doc.toJson();
    out.flush();
    file.close();
    return true;
}

void SettingsDialog::initLineEdit()
{
    m_ip = new QLineEdit(this);
    QRegExp ipRx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    m_ip->setValidator(new QRegExpValidator(ipRx));
    m_ip->setMinimumHeight(26);
    m_port = new QSpinBox(this);
    m_port->setRange(1000,20000);
    m_port->setMinimumHeight(26);

    QVBoxLayout *mianLayout = new QVBoxLayout();
    QFormLayout *formLayout = new QFormLayout();
    QPushButton *okBtn = new QPushButton(tr("保存"),this);
    mianLayout->addLayout(formLayout);
    mianLayout->addWidget(okBtn,0,Qt::AlignRight);
    mianLayout->setSpacing(10);
    setLayout(mianLayout);

    formLayout->addRow(tr("服务器IP:"),m_ip);
    formLayout->addRow(tr("服务器Port:"),m_port);
    formLayout->setSpacing(10);

    connect(okBtn, &QPushButton::clicked, [=]{
        if (!ipRx.exactMatch(m_ip->text()))
        {
            QMessageBox::warning(this, tr("警告"), tr("IP地址不合法"));
        }
        else
        {
            this->accept();
        }
    });

    m_ip->setText(m_config.value(CONFIG_JSON_HTTP_SERVER_IP).toString());
    m_port->setValue(m_config.value(CONFIG_JSON_HTTP_SERVER_PORT).toString().toInt());
}
