#include "HttpClient.h"
#include <QDebug>

#include <QJsonDocument>
#include <QJsonParseError>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QTextCodec>
#include <QEventLoop>
#include <QFile>
#include <QSound>
#include <QTcpSocket>

#include "common_define.h"

HttpClient::HttpClient()
{
    //读取配置文件
    readConfig();

    initPlayThread();
}

HttpClient::~HttpClient()
{
    m_playOkThread.quit();
    m_playOkThread.wait();

    m_playNoThread.quit();
    m_playNoThread.wait();

    m_playErrorThread.quit();
    m_playErrorThread.wait();

    qDebug() << "HttpClient::~HttpClient";
}

void HttpClient::hanldeAsrResult(const std::string &asrResult)
{
//    qDebug() << "hanldeAsrResult currentThread:" << QThread::currentThread();

    if (!isNetworkAvailable())
    {
        qDebug() << "isNetworkAvailable = false";
        emit signal_playError();
        return;
    }

    readConfig();

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

void HttpClient::handleAsrError()
{
    emit signal_playNo();
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
//    qDebug() << "syncPost currentThread:" << QThread::currentThread();

    static QNetworkAccessManager m_networkMgr;
    QEventLoop loop;

    QNetworkReply *reply = m_networkMgr.post(request,body);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    QByteArray replyData;
    if(reply->error() == QNetworkReply::NoError)
    {
        qDebug() << reply->readAll();
        emit signal_playOk();
    }
    else
    {
        qDebug() << "reply error = " << reply->errorString();
        emit signal_playError();
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

void HttpClient::initPlayThread()
{
    QSound *okSound = new QSound(":/audio/audio/auido_ok.wav");
    QSound *noSound = new QSound(":/audio/audio/auido_no.wav");
    QSound *errorSound = new QSound(":/audio/audio/auido_error.wav");

    okSound->moveToThread(&m_playOkThread);
    noSound->moveToThread(&m_playNoThread);
    errorSound->moveToThread(&m_playErrorThread);

    connect((&m_playOkThread), &QThread::finished, okSound, &QSound::deleteLater);
    connect((&m_playNoThread), &QThread::finished, noSound, &QSound::deleteLater);
    connect((&m_playErrorThread), &QThread::finished, errorSound, &QSound::deleteLater);

    connect(this, SIGNAL(signal_playOk()), okSound, SLOT(play()));
    connect(this, SIGNAL(signal_playNo()), noSound, SLOT(play()));
    connect(this, SIGNAL(signal_playError()), errorSound, SLOT(play()));

    m_playOkThread.start();
    m_playNoThread.start();
    m_playErrorThread.start();
}

bool HttpClient::isNetworkAvailable()
{
    static QTcpSocket socket;
    socket.abort();
    socket.connectToHost(m_config.value(CONFIG_JSON_HTTP_SERVER_IP).toString(),
                         m_config.value(CONFIG_JSON_HTTP_SERVER_PORT).toString().toInt());
    return socket.waitForConnected(10000);
}
