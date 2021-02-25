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

    QString cmd = convert2Cmd(asrResult);
    if (cmd.isEmpty())
    {
        qDebug() << "convert to cmd failed!";
        return;
    }

    QJsonObject obj;
    obj.insert(HTTP_BODY_CMD,cmd);
    obj.insert(HTTP_BODY_ASR, QString::fromStdString(asrResult));
    QJsonDocument doc(obj);

    qDebug() << obj;

//    this->syncGet(request);
    this->syncPost(request,doc.toJson());
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
        QTextCodec *codec = QTextCodec::codecForName("GBK");
        qDebug() << codec->toUnicode(reply->readAll());
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
    request.setRawHeader("Content-Type","application/json;charset=UTF-8");
    request.setRawHeader("Accept", "application/json;charset=UTF-8");
    return request;
}

QString HttpClient::getBaseUrl()
{
    QString url = QString("http://%1:%2").
            arg(m_config.value(CONFIG_JSON_HTTP_SERVER_IP).toString()).
            arg(m_config.value(CONFIG_JSON_HTTP_SERVER_PORT).toString().toInt());
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
#if 1
    QNetworkRequest request = getBaseRequest();
    QString baseUrl = getBaseUrl();
    QString userInput = QString("%1").arg(baseUrl);
    QUrl url = QUrl::fromUserInput(userInput);
    request.setUrl(url);
#endif

#if 0
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

QString HttpClient::convert2Cmd(const std::string &asrResult)
{
    QString cmd;

    QJsonArray array = m_config.value(CONFIG_JSON_ASR_CMD).toArray();
    QJsonArray::iterator itor = array.begin();

    while (itor != array.end()) {
        QJsonObject obj = itor->toObject();
        QStringList keys = obj.keys();
        if (obj.value(keys.first()).toString() == QString::fromStdString(asrResult))
        {
            cmd = keys.first();
            break;
        }
        itor++;
    }

    return cmd;
}
