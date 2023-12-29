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
#include "src/daemon/log/manager.h"

class QWaitCondition;
class QReadWriteLock;
class QMutex;
class QFile;
class Log;

namespace KS
{
namespace Log
{
class WriteWorker : public QThread
{
public:
    explicit WriteWorker(Manager* logManager)
        : QThread(logManager),
          m_logManager(logManager)
    {
    }
    void run() override;

    void stop();

private:
    Manager* m_logManager;
    QMutex* m_condition;
    bool m_isStop = false;
};
};  // namespace Log
};  // namespace KS