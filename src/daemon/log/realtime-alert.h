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
#include <QSocketNotifier>

template <typename T> class QList;
class QTimer;
struct ipset;
struct ipset_session;

namespace KS
{
namespace Log
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
    static int ipsetGetDataCB(struct ipset_session* session, void* p, const char* fmt, ...) __attribute__((format(printf, 3, 4)));
    static int ipsetCustomErrorCB(struct ipset* ipset, void* p, int status, const char* fmt, ...) __attribute__((format(printf, 4, 5)));
    static int ipsetStandardErrorCB(ipset* ipset, void* p);

private slots:
    void processAuditData(int socket);
    void processIPSetData();

private:
    QSocketNotifier* m_auditNotifier;
    // ipset 无法注册回调来处理数据，所以用定时器定时轮循处理
    QTimer* m_nmapDetectTimer;
    QString* m_ipsetData;
    ipset* m_ipset;
};

};  // namespace Log
};  // namespace KS