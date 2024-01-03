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

#pragma once

#include <QObject>
#include <QProcess>
#include <QSocketNotifier>

template <typename T> class QList;
class QTimer;
class QProcess;

namespace KS
{
namespace ToolBox
{
struct AuditLogRecord;

class RealTimeAlert : public QObject
{
    Q_OBJECT
    using AuditLogEvent = QList<AuditLogRecord>;

public:
    RealTimeAlert();
    virtual ~RealTimeAlert();

private:
    bool initAuditReceiver();
    bool initIPSetMonitor();
    QList<AuditLogEvent> parserAudit(const char* audit_log);

private slots:
    void processAuditData(int socket);
    void processIPSetData();
    void getIPSetData(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QSocketNotifier* m_auditNotifier;
    // ipset 无法注册回调来处理数据，所以用定时器定时轮循处理
    QTimer* m_nmapDetectTimer;
    QString m_ipsetData;
    QProcess* m_getIPSetDataProcess;
    QProcess* m_clearIPSetDataProcess;
};

};  // namespace Log
};  // namespace KS