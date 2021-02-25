#include "GCSClientMgr.h"
#include "common_define.h"
#include <QThread>

GCSClientMgr::GCSClientMgr():
    m_stop(false)
{
    this->startThread();
}

GCSClientMgr::~GCSClientMgr()
{
    this->stopThread();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
    qDebug() << "GCSClientMgr::~GCSClientMgr";
}

void GCSClientMgr::asrResult(const std::string &result)
{
    qDebug() << "asrResult currentThread:" << QThread::currentThread();

    std::unique_lock<std::mutex> lck(m_mtx);
    m_deque.push_back(result);
    m_cond.notify_one();
}

void GCSClientMgr::asrRrror()
{
    qDebug() << "asrResult asrRrror:" << QThread::currentThread();

    m_httpPtr->handleAsrError();
}

void GCSClientMgr::startThread()
{
    m_thread = std::thread(std::bind(&GCSClientMgr::runThread, this));
}

void GCSClientMgr::stopThread()
{
    std::unique_lock<std::mutex> lck(m_mtx);
    m_stop = true;
    m_cond.notify_one();
}

void GCSClientMgr::runThread()
{
    qDebug() << "runThread currentThread:" << QThread::currentThread();

    m_httpPtr = std::make_shared<HttpClient>();

    while (!m_stop)
    {
        std::unique_lock<std::mutex> lck(m_mtx);
        if (m_deque.empty() && !m_stop)
        {
            qDebug() << "*****GCSClientMgr wait!*****";
            m_cond.wait(lck);
        }

        if (m_stop)
        {
            qDebug() << "*****GCSClientMgr quit thread!*****";
            break;
        }

        std::string asrResult = m_deque.front();
        m_deque.pop_front();
        m_httpPtr->hanldeAsrResult(asrResult);
    }
}
