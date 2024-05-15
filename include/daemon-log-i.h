/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QObject>
#include <QString>

namespace KS
{
class IDaemonLog;
extern IDaemonLog *g_logManager;

#define SSR_LOG(logType, logMsg, result, dbusID)                 \
    {                                                            \
        g_logManager->writeLog(logType, logMsg, result, dbusID); \
    }

#define SSR_LOG_SUCCESS(logType, logMsg, dbusID)               \
    {                                                          \
        g_logManager->writeLog(logType, logMsg, true, dbusID); \
    }

#define SSR_LOG_ERROR(logType, logMsg, dbusID)                  \
    {                                                           \
        g_logManager->writeLog(logType, logMsg, false, dbusID); \
    }

// TODO: 不应该叫日志类型，应该叫日志分类，后面要修改名称
enum LogType
{
    ERROR = -1,
    DEVICE = (1 << 0),
    TOOL_BOX = (1 << 1),
    BASELINE_REINFORCEMENT = (1 << 2),
    TRUSTED_PROTECTION = (1 << 3),
    FILES_PROTECTION = (1 << 4),
    PRIVATE_BOX = (1 << 5),
    ACCOUNT = (1 << 6),
    AVC = (1 << 7)
};

class IDaemonLog
{
public:
    virtual ~IDaemonLog(){};

    virtual void writeLog(LogType logType, const QString &logMsg, bool result, const QString &dbusID) = 0;
    virtual void writeLog(const QString &name, int role, QDateTime timestamp, LogType logType, bool result, const QString &logMsg) = 0;
};
}  // namespace KS
