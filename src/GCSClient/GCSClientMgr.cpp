#include "GCSClientMgr.h"

GCSClientMgr::GCSClientMgr():
    m_stop(false),
    m_httpPtr(std::make_shared<HttpClient>())
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
    std::unique_lock<std::mutex> lck(m_mtx);
    m_deque.push_back(result);
    m_cond.notify_one();
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
    while (!m_stop)
    {
        std::unique_lock<std::mutex> lck(m_mtx);
        if (m_deque.empty() && !m_stop)
        {
            qDebug() << "GCSClientMgr wait!";
            m_cond.wait(lck);
        }

        if (m_stop)
        {
            qDebug() << "GCSClientMgr quit thread!";
            break;
        }

        std::string asrResult = m_deque.front();
        m_deque.pop_front();
        m_httpPtr->hanldeAsrResult(asrResult);
        lck.unlock();

        qDebug() << "线程处理 asrResult = " << asrResult.c_str();
    }
}
