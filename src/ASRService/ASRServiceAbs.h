#ifndef ASRSERVICEABS_H
#define ASRSERVICEABS_H

#include <string>

class ASRServiceAbs :public std::enable_shared_from_this<ASRServiceAbs>
{
public:
    ASRServiceAbs();
    virtual ~ASRServiceAbs();

    virtual bool startService() = 0;
    virtual bool stopService() = 0;
    virtual std::string errorString() = 0;
};

typedef std::shared_ptr<ASRServiceAbs> ASRServicePtr;

#endif // ASRSERVICEABS_H
