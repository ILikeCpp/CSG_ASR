#include "HttpClient.h"
#include <QDebug>

#include <QJsonDocument>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

HttpClient::HttpClient()
{
    m_networkMgr = new QNetworkAccessManager(this);
    connect(m_networkMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(onRequestFinished(QNetworkReply*)));
}

HttpClient::~HttpClient()
{
    qDebug() << "HttpClient::~HttpClient";
}

void HttpClient::hanldeAsrResult(const std::string &asrResult)
{
    qDebug() << "hanldeAsrResult " << asrResult.c_str();

//    QNetworkRequest request = this->getNetworkRequest();

//    QJsonObject obj;
//    QJsonDocument doc(obj);

//    qDebug() << m_networkMgr->post(request,doc.toJson());
    qDebug() << m_networkMgr->get(QNetworkRequest(QUrl("https://www.baidu.com/")));
}

void HttpClient::onRequestFinished(QNetworkReply *reply)
{
    qDebug() << reply->readAll();
    if(nullptr == reply)
    {
        qDebug()<<"HttpClient::onRequestFinished reply is nullptr";
        return;
    }

    if(reply->error() == QNetworkReply::NoError)
    {
        //处理接收的处理，然后post出去
        QByteArray data = reply->readAll();
        qDebug() << "reply===" << data;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        QJsonObject resultObj = jsonDoc.object();
        qDebug() << "resultObj===" << resultObj;
    }

    // 处理完成，删除，不能直接delete.
    reply->deleteLater();
}

QNetworkRequest HttpClient::getBaseRequest()
{
    QNetworkRequest request;
//    request.setRawHeader("Content-Type","application/x-www-form-urlencoded"/*"application/json;charset=UTF-8"*/);
    request.setRawHeader("Accept", "application/json");
    return request;
}

QString HttpClient::getBaseUrl()
{
    QString url = QString("http://%1:%2").arg("127.0.0.1").arg("8081");
    return url;
}

QNetworkRequest HttpClient::getNetworkRequest()
{
    QNetworkRequest request = getBaseRequest();
    QString baseUrl = getBaseUrl();
    QString userInput = QString("%1/scwp/gcs/startPatrolJob").arg(baseUrl);
    QUrl url = QUrl::fromUserInput(userInput);
    request.setUrl(url);
    return request;
}
