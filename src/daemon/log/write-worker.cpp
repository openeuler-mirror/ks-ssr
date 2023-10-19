#include "write-worker.h"

namespace KS
{
namespace Log
{

void WriteWorker::run()
{
    m_condition = new QMutex();
    while (true)
    {
        m_waitCondition->wait(m_condition);
        if (m_isStop)
        {
            break;
        }
        QString log;
        QTextStream logStream(&log);
        m_queueMutex->lock();
        while (!m_messageQueue->isEmpty())
        {
            logStream << m_messageQueue->dequeue() << '\n';
        }
        m_queueMutex->unlock();
        m_fileMutex->lock();
        m_file->write(log.toLocal8Bit());
        m_file->flush();
        m_fileMutex->unlock();
    }
}
void WriteWorker::stop()
{
    QMutexLocker lock(m_condition);
    m_isStop = true;
    m_waitCondition->notify_one();
}
};  // namespace Log
};  // namespace KS