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

#include "src/daemon/log/write-worker.h"
#include <QFile>
#include <QMutexLocker>
#include <QQueue>
#include <QTextStream>
#include <QWaitCondition>

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