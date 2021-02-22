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

public slots:
    void onRequestFinished(QNetworkReply *reply);

private:
    QNetworkRequest getNetworkRequest();
    QNetworkRequest getBaseRequest();
    QString getBaseUrl();

private:
    QNetworkAccessManager *m_networkMgr;
};

typedef std::shared_ptr<HttpClient> HttpClientPtr;

#endif // HTTPCLIENT_H
