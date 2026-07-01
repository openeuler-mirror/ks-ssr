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
#include <QReadWriteLock>
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
        m_logManager->m_waitCondition->wait(m_condition);
        if (m_isStop)
        {
            break;
        }
        QString log;
        // 现将日志 copy 到本地变量中， 然后释放临界资源再序列化本地变量写入文件。
        static QList<Log> tmpList;
        QTextStream logStream(&log);
        m_logManager->m_listMutex.lockForRead();
        while (m_logManager->m_firstNeedWrite != static_cast<uint>(m_logManager->m_logList.size()))
        {
            tmpList << m_logManager->m_logList.at(m_logManager->m_firstNeedWrite);
            m_logManager->m_firstNeedWrite++;
        }
        m_logManager->m_listMutex.unlock();
        m_logManager->m_fileMutex.lock();
        while (tmpList.size() > 0)
        {
            if (m_logManager->m_fileLine >= m_logManager->m_configurations.m_maxLogFileLine)
            {
                emit m_logManager->needLogRotate();
                break;
            }
            m_logManager->m_fileLine++;
            auto firstLog = tmpList.takeFirst();
            logStream << Message::serialize(firstLog) << '\n';
        }
        logStream.flush();
        m_logManager->m_file->write(log.toLocal8Bit());
        m_logManager->m_file->flush();
        m_logManager->m_fileMutex.unlock();
    }
}
void WriteWorker::stop()
{
    QMutexLocker lock(m_condition);
    m_isStop = true;
    m_logManager->m_waitCondition->notify_one();
}
};  // namespace Log
};  // namespace KS