/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#include <QThread>

class QFile;
class QWaitCondition;
class QMutex;
template <typename T> class QQueue;

namespace KS
{
namespace Log
{
class WriteWorker : public QThread
{
public:
    explicit WriteWorker(QQueue<QString>* queue, QFile* logFile, QWaitCondition* waitCondition, QMutex* writeQueue, QMutex* writeFile, QObject* parent = nullptr)
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