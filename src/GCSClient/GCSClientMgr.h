#ifndef GCSCLIENTMGR_H
#define GCSCLIENTMGR_H

#include "ASRListennerAbs.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>

class GCSClientMgr :public ASRListennerAbs
{
public:
    GCSClientMgr();
    ~GCSClientMgr();

    virtual void asrResult(const std::string &result);

private:
    void startThread();
    void stopThread();
    void runThread();

private:
    std::thread m_thread;
    std::mutex m_mtx;
    std::condition_variable m_cond;
    std::deque<std::string> m_deque;
    bool m_stop;
};

#endif // GCSCLIENTMGR_H
