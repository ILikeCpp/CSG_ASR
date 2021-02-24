#include "HttpClient.h"
#include <QDebug>

#include <QJsonDocument>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QTextCodec>
#include <QEventLoop>
#include <QFile>

#include "common_define.h"

HttpClient::HttpClient()
{
    //读取配置文件
    readConfig();
}

HttpClient::~HttpClient()
{
    qDebug() << "HttpClient::~HttpClient";
}

void HttpClient::hanldeAsrResult(const std::string &asrResult)
{
    QNetworkRequest request = this->getNetworkRequest();

    int cmd = convert2Cmd(asrResult);
    if (-1 == cmd)
    {
        qDebug() << "convert to cmd failed!";
        return;
    }

    QJsonObject obj;
    obj.insert(HTTP_BODY_CMD,cmd);
    obj.insert(HTTP_BODY_ASR, QString::fromStdString(asrResult));
    obj.insert(HTTP_BODY_ROBOT_IP,m_config.value(CONFIG_JSON_ROBOT_IP).toString());
    obj.insert(HTTP_BODY_ROBOT_PORT,m_config.value(CONFIG_JSON_ROBOT_PORT).toString().toInt());
    QJsonDocument doc(obj);

    qDebug() << obj;

//    this->syncGet(request);
//    this->syncPost(request,doc.toJson());
}

void HttpClient::onRequestFinished(QNetworkReply *reply)
{
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

QByteArray HttpClient::syncGet(QNetworkRequest request)
{
    static QNetworkAccessManager m_networkMgr;
    QEventLoop loop;

    QNetworkReply *reply = m_networkMgr.get(request);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    QByteArray replyData;
    if(reply->error() == QNetworkReply::NoError)
    {
        qDebug() << reply->readAll();
    }
    else
    {
        qDebug() << "reply error = " << reply->errorString();
    }

    reply->deleteLater();
    reply = nullptr;

    return replyData;
}

QByteArray HttpClient::syncPost(QNetworkRequest request,const QByteArray &body)
{
    static QNetworkAccessManager m_networkMgr;
    QEventLoop loop;

    QNetworkReply *reply = m_networkMgr.post(request,body);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    QByteArray replyData;
    if(reply->error() == QNetworkReply::NoError)
    {
        qDebug() << reply->readAll();
    }
    else
    {
        qDebug() << "reply error = " << reply->errorString();
    }

    reply->deleteLater();
    reply = nullptr;

    return replyData;
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

void HttpClient::readConfig()
{
    QFile file("../../data/config/config.json");
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        qDebug() << "file open failed! " << file.errorString();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    m_config = doc.object();
    file.close();

//    qDebug() << m_config;
}

QNetworkRequest HttpClient::getNetworkRequest()
{
#if 0
    QNetworkRequest request = getBaseRequest();
    QString baseUrl = getBaseUrl();
    QString userInput = QString("%1/scwp/gcs/asrControl").arg(baseUrl);
    QUrl url = QUrl::fromUserInput(userInput);
    request.setUrl(url);
#endif

#if 1
    QNetworkRequest request;
    QUrl url("https://tcc.taobao.com/cc/json/mobile_tel_segment.htm?");
    QUrlQuery query;
    query.addQueryItem("tel","15850890324");
    url.setQuery(query);
    request.setUrl(url);
    //设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/javascript;charset=GBK");
#endif

    return request;
}

int HttpClient::convert2Cmd(const std::string &asrResult)
{
    int cmd = -1;

    QJsonArray array = m_config.value(CONFIG_JSON_ASR_CMD).toArray();
    QJsonArray::iterator itor = array.begin();
    int key = 1;

    while (itor != array.end()) {
        QJsonObject obj = itor->toObject();
        if (obj.value(QString::number(key)).toString() == QString::fromStdString(asrResult))
        {
            cmd = key;
            break;
        }
        key++;
        itor++;
    }

    return cmd;
}
