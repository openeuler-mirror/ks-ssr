#include <QThread>
#include <QFile>
#include <QWaitCondition>
#include <QMutexLocker>
#include <QTextStream>
#include <QQueue>

namespace KS
{
namespace Log
{
class WriteWorker : public QThread
{
public:
    explicit WriteWorker(QQueue<QString>* queue, QFile* logFile,
                         QWaitCondition* waitCondition, QMutex* writeQueue, QMutex* writeFile, QObject* parent = nullptr)
        : QThread(parent),
          m_messageQueue(queue),
          m_file(logFile),
          m_waitCondition(waitCondition),
          m_queueMutex(writeQueue),
          m_fileMutex(writeFile)
    {
    }
    void run() override;

    void stop();

private:
    QQueue<QString>* m_messageQueue;
    QFile* m_file;
    QWaitCondition* m_waitCondition;
    QMutex* m_condition;
    QMutex* m_queueMutex;
    QMutex* m_fileMutex;
    bool m_isStop = false;
};
};  // namespace Log
};  // namespace KS