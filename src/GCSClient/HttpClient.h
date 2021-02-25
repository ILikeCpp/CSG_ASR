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
    void handleAsrError();

signals:
    void signal_playOk();
    void signal_playNo();
    void signal_playError();

private:
    QByteArray syncGet(QNetworkRequest request);
    QByteArray syncPost(QNetworkRequest request,const QByteArray &body);
    QNetworkRequest getNetworkRequest();
    QNetworkRequest getBaseRequest();
    QString getBaseUrl();

    void readConfig();
    QString convert2Cmd(const std::string &asrResult);
    void initPlayThread();
    bool isNetworkAvailable();

private:
    QJsonObject m_config;
    QThread m_playOkThread;
    QThread m_playNoThread;
    QThread m_playErrorThread;
};

typedef std::shared_ptr<HttpClient> HttpClientPtr;

#endif // HTTPCLIENT_H
