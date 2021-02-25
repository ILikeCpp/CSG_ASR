#ifndef ASRLISTENNERABS_H
#define ASRLISTENNERABS_H

#include <string>
#include <QDebug>

class ASRListennerAbs :public std::enable_shared_from_this<ASRListennerAbs>
{
public:
    ASRListennerAbs() {}
    virtual ~ASRListennerAbs() {
        qDebug() << "ASRListennerAbs::~ASRListennerAbs";
    }

    virtual void asrResult(const std::string &result) = 0;
    virtual void asrRrror() = 0;
};

typedef std::shared_ptr<ASRListennerAbs> ASRListennerPtr;

#endif // ASRLISTENNERABS_H
