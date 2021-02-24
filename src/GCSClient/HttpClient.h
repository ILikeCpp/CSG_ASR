#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <string>
#include <QObject>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>

class QNetworkAccessManager;
class QNetworkReply;

class HttpClient :public QObject
{
    Q_OBJECT
public:
    HttpClient();
    ~HttpClient();

    void hanldeAsrResult(const std::string &asrResult);

private:
    QByteArray syncGet(QNetworkRequest request);
    QByteArray syncPost(QNetworkRequest request,const QByteArray &body);
    QNetworkRequest getNetworkRequest();
    QNetworkRequest getBaseRequest();
    QString getBaseUrl();

    void readConfig();
    int convert2Cmd(const std::string &asrResult);

private:
    QJsonObject m_config;
};

typedef std::shared_ptr<HttpClient> HttpClientPtr;

#endif // HTTPCLIENT_H
