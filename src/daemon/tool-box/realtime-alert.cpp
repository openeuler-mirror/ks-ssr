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

#include "src/daemon/tool-box/realtime-alert.h"
#include <auparse.h>
#include <libaudit.h>
#include <linux/un.h>
#include <qt5-log-i.h>
#include <unistd.h>
#include <QDateTime>
#include <QDir>
#include <QList>
#include <QProcess>
#include <QSocketNotifier>
#include <QTimer>
#include "include/ssr-i.h"
#include "include/ssr-marcos.h"
#include "realtime-alert.h"
#include "src/daemon/account/manager.h"
#include "src/daemon/tool-box/manager.h"

#define SOCKET_PATH "/var/run/audispd_events"
#define KS_SSR_AUDIT_KEYWORD "(ks-ssr)"
/*
ipset 的数据格式如下，add ks-ssr-ip-set 后面接的是 nmap 使用者的 ip 地址
create ks-ssr-ip-set hash:ip family inet hashsize 1024 maxelem 65536
add ks-ssr-ip-set 127.0.0.1
*/
#define IPSET_NMAP_IP_KEYWORD "add ks-ssr-ip-set "
#define IPSET_CMD "ipset"
#define IPSET_CREATE_SSR_SET "create ks-ssr-ip-set hash:ip"
#define IPSET_GET_DATA "list ks-ssr-ip-set -o save"
#define IPSET_CLEAR_DATA "flush ks-ssr-ip-set"

#define IPTABLES_CMD "iptables"
#define IPTABLES_CREATE_SSR_RULE "-A INPUT -p tcp --syn ! --dport 22 -j SET --add-set ks-ssr-ip-set src"
#define IPTABLES_DELETE_SSR_RULE "-D INPUT -p tcp --syn ! --dport 22 -j SET --add-set ks-ssr-ip-set src"

#pragma message("审计告警功能需要将 /etc/audit/plugins.d/af_unix.conf 中 active 字段设置为 yes 并重启 auditd")
#pragma message("软件被移除时执行 iptables -D INPUT -p tcp --syn ! --dport 22 -j SET --add-set ks-ssr-ip-set src && ipset destroy ks-ssr-ip-set")
#pragma message("此功能应该要有开关，方便用户开启或关闭")
namespace KS
{
namespace ToolBox
{
struct AuditLogRecord
{
    QString type;
    QDateTime timeStamp;
    QMap<QString, QString> field;
};

RealTimeAlert::RealTimeAlert()
    : m_nmapDetectTimer(new QTimer(this)),
      m_getIPSetDataProcess(new QProcess()),
      m_clearIPSetDataProcess(new QProcess())
{
#pragma message("审计规则应该由接口配置，而不是写死在配置文件中")
    if (!initAuditReceiver())
    {
        KLOG_ERROR() << "Failed to init audit receiver, realtime alert feature is diabled";
    }

    if (!initIPSetMonitor())
    {
        KLOG_ERROR() << "Failed to init ipset, nmap detection feature is diabled";
    }
}

RealTimeAlert::~RealTimeAlert()
{
}

bool RealTimeAlert::initAuditReceiver()
{
    // 重新加载审计规则
    if (QProcess::execute("/usr/sbin/augenrules", QStringList("--load")) != 0)
    {
        KLOG_WARNING() << "Failed to update audit rules!";
    }
    QDir dir;
    if (!dir.exists(SOCKET_PATH))
    {
        KLOG_WARNING() << "The Socket: " << SOCKET_PATH << " Socket does not exist!";
        return false;
    }
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SOCKET_PATH);
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        KLOG_WARNING() << "Failed to create audit socket";
        return false;
    }

    if (::connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        KLOG_WARNING() << "Failed to connect auditd socket";
        close(sockfd);
        return false;
    }
    m_auditNotifier = new QSocketNotifier(sockfd, QSocketNotifier::Type::Read, this);
    QObject::connect(m_auditNotifier, &QSocketNotifier::activated, this, &RealTimeAlert::processAuditData);
    return true;
}

bool RealTimeAlert::initIPSetMonitor()
{
    // 由于基线加固的对 iptables 的操作会清空 iptables 的规则， 所以这里手动创建
    if (0 != QProcess::execute(IPSET_CMD, QString(IPSET_CREATE_SSR_SET).split(' ')))
    {
        KLOG_INFO() << "Failed to create ipset set: set with the same name already exists";
    }
    QProcess::execute(IPTABLES_CMD, QString(IPTABLES_DELETE_SSR_RULE).split(' '));
    if (0 != QProcess::execute(IPTABLES_CMD, QString(IPTABLES_CREATE_SSR_RULE).split(' ')))
    {
        KLOG_ERROR() << "Failed to add iptables rule, nmap attack detection is disable";
        return false;
    }
    // 流程参考 ipset 源码
    m_getIPSetDataProcess->setProgram(IPSET_CMD);
    m_getIPSetDataProcess->setArguments(QString(IPSET_GET_DATA).split(' '));
    QObject::connect(m_getIPSetDataProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
                     this, SLOT(getIPSetData(int, QProcess::ExitStatus)));
    m_clearIPSetDataProcess->setProgram(IPSET_CMD);
    m_clearIPSetDataProcess->setArguments(QString(IPSET_CLEAR_DATA).split(' '));
    m_nmapDetectTimer->setInterval(5 * 1000);
    m_nmapDetectTimer->start();
    QObject::connect(m_nmapDetectTimer, &QTimer::timeout, this, &RealTimeAlert::processIPSetData);
    return true;
}

/*

----
type=PROCTITLE msg=audit(11/24/2023 15:20:31.209:145326) : proctitle=vim /home/wangyucheng/workspace/token
type=PATH msg=audit(11/24/2023 15:20:31.209:145326) : item=0 name=/home/wangyucheng/workspace/token inode=1078004315 dev=08:01 mode=file,664 ouid=wangyucheng ogid=wangyucheng rdev=00:00 nametype=NORMAL cap_fp=none cap_fi=none cap_fe=0 cap_fver=0 cap_frootid=0
type=CWD msg=audit(11/24/2023 15:20:31.209:145326) : cwd=/home/wangyucheng/workspace/ks-ssr/build
type=SYSCALL msg=audit(11/24/2023 15:20:31.209:145326) : arch=x86_64 syscall=openat success=yes exit=3 a0=0xffffff9c a1=0x557896647ce0 a2=O_RDONLY a3=0x0 items=1 ppid=28656 pid=428220 auid=wangyucheng uid=wangyucheng gid=wangyucheng euid=wangyucheng suid=wangyucheng fsuid=wangyucheng egid=wangyucheng sgid=wangyucheng fsgid=wangyucheng tty=pts1 ses=3 comm=vim exe=/usr/bin/vim key=ks-ssrqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq
----
type=PROCTITLE msg=audit(11/24/2023 15:20:31.209:145327) : proctitle=vim /home/wangyucheng/workspace/token
type=PATH msg=audit(11/24/2023 15:20:31.209:145327) : item=0 name=/home/wangyucheng/workspace/token inode=1078004315 dev=08:01 mode=file,664 ouid=wangyucheng ogid=wangyucheng rdev=00:00 nametype=NORMAL cap_fp=none cap_fi=none cap_fe=0 cap_fver=0 cap_frootid=0
type=CWD msg=audit(11/24/2023 15:20:31.209:145327) : cwd=/home/wangyucheng/workspace/ks-ssr/build
type=SYSCALL msg=audit(11/24/2023 15:20:31.209:145327) : arch=x86_64 syscall=openat success=yes exit=3 a0=0xffffff9c a1=0x557896647ce0 a2=O_RDONLY a3=0x0 items=1 ppid=28656 pid=428220 auid=wangyucheng uid=wangyucheng gid=wangyucheng euid=wangyucheng suid=wangyucheng fsuid=wangyucheng egid=wangyucheng sgid=wangyucheng fsgid=wangyucheng tty=pts1 ses=3 comm=vim exe=/usr/bin/vim key=ks-ssrqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq

上面的 audit log 中，有两个事件， 此处监听 "/var/run/audispd_events" 套接字， 每次可写时代表一个事件， 所以每次调用此函数时只有一个事件， 事件中可以有多个 record， type 开头， 每一行为一个 record，每条 record 中有多个 field， field 以空格分割，当 record 中有 key 为 "ks-ssr" 开头时就认为关注的事件发生， 故发送 HazardDetected 信号。
*/

QList<RealTimeAlert::AuditLogEvent> RealTimeAlert::parserAudit(const char *audit_log)
{
    QList<AuditLogEvent> logEventList;
    auto au = auparse_init(AUSOURCE_BUFFER, audit_log);
    auparse_first_record(au);
    do
    {
        AuditLogEvent logRecordList;
        do
        {
            AuditLogRecord log;
            char buf[32];
            const char *type = auparse_get_type_name(au);
            if (type == NULL)
            {
                snprintf(buf, sizeof(buf), "%d", auparse_get_type(au));
                type = buf;
            }
            log.type = type;
            log.timeStamp = QDateTime::fromSecsSinceEpoch(auparse_get_time(au));
            do
            {
                const char *name = auparse_get_field_name(au);
                const char *value = auparse_get_field_str(au);
                log.field.insert(name, value);
            } while (auparse_next_field(au) > 0);
            logRecordList.append(log);
        } while (auparse_next_record(au) > 0);
        logEventList.append(logRecordList);
    } while (auparse_next_event(au) > 0);
    auparse_destroy(au);
    return logEventList;
}

void RealTimeAlert::getIPSetData(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit || exitCode != 0)
    {
        KLOG_WARNING() << "Failed to get IPSet data, exitCode: " << exitCode
                       << ", error msg: " << this->m_getIPSetDataProcess->readAllStandardError();
    }
    m_ipsetData = QString::fromLocal8Bit(this->m_getIPSetDataProcess->readAllStandardOutput());
}

void RealTimeAlert::processIPSetData()
{
    m_getIPSetDataProcess->start();
    m_getIPSetDataProcess->waitForFinished();
    RETURN_IF_TRUE(m_ipsetData.isNull() || m_ipsetData.isEmpty());
    QStringList nmapAttackers;
    for (const auto &line : m_ipsetData.split('\n'))
    {
        CONTINUE_IF_TRUE(!line.startsWith(IPSET_NMAP_IP_KEYWORD));
        auto nmapAttackerIp = line.mid(sizeof(IPSET_NMAP_IP_KEYWORD) - 1);
        CONTINUE_IF_TRUE(nmapAttackerIp == "127.0.0.1" || nmapAttackerIp == "localhost");
        nmapAttackers.append(nmapAttackerIp);
    }
    RETURN_IF_TRUE(nmapAttackers.isEmpty());
    KLOG_DEBUG() << "Detect nmap attack, attacker ips: " << nmapAttackers;
    Manager::hazardDetected(ATTACK_DETECT, nmapAttackers.join(','));
    KS::Log::Manager::writeLog({"secadm",
                                Account::Manager::AccountRole::secadm,
                                QDateTime::currentDateTime(),
                                KS::Log::Manager::LogType::TOOL_BOX,
                                true,
                                tr("Detected nmap attack! attacker ip:%1").arg(nmapAttackers.join(','))});
    m_ipsetData.clear();
    m_clearIPSetDataProcess->start();
}

void RealTimeAlert::processAuditData(int socket)
{
    char data[MAX_AUDIT_MESSAGE_LENGTH + 1] = {0};
    auto len = read(socket, data, MAX_AUDIT_MESSAGE_LENGTH);
    if (len < 0)
    {
        KLOG_WARNING() << "Failed to read socket: " << socket;
        return;
    }
    auto logEventList = parserAudit(data);
    QStringList alertMsgList;
    for (const auto &logEvent : logEventList)
    {
        QMap<QString, QMap<QString, QString>> allAuditRecordMap{};
        QMap<QString, QStringList> allAuditRecordList{};
        for (const auto &logRecord : logEvent)
        {
            auto recordType = logRecord.type;
            allAuditRecordList.insert(recordType, {});
            for (const auto &record : logRecord.field.keys())
            {
                allAuditRecordList[recordType] << QString("%1=%2").arg(record).arg(logRecord.field.value(record));
            }
            allAuditRecordMap.insert(recordType, logRecord.field);
            // 将 系统审计 加入日志
            if (recordType.contains("AVC"))
            {
                QString logMsg("PROCTITLE: %1,SYSCALL: %2,AVC: %3");
                KS::Log::Manager::writeLog({"audadm",
                                            Account::Manager::AccountRole::audadm,
                                            logRecord.timeStamp,
                                            KS::Log::Manager::LogType::AVC,
                                            allAuditRecordMap["SYSCALL"]["success"] == "yes",
                                            allAuditRecordList[recordType].join(',')});
            }
            if (recordType == "SYSCALL")
            {
                CONTINUE_IF_TRUE(!logRecord.field.contains("key"));
                CONTINUE_IF_TRUE(!logRecord.field.value("key").contains(KS_SSR_AUDIT_KEYWORD));
                alertMsgList.append(QString("uid=%1, exe=%2, op=%3")
                                        .arg(logRecord.field.value("uid"))
                                        .arg(logRecord.field.value("exe"))
                                        .arg(logRecord.field.value("syscall")));
            }
            // 避免一次审计事件输出多条告警
            break;
        }
        if (!alertMsgList.isEmpty())
        {
            break;
        }
    }
    RETURN_IF_TRUE(alertMsgList.isEmpty());
    Manager::hazardDetected(HAZARD_BEHAVIOR, alertMsgList.join(','));
    KS::Log::Manager::writeLog({"secadm",
                                Account::Manager::AccountRole::secadm,
                                QDateTime::currentDateTime(),
                                KS::Log::Manager::LogType::TOOL_BOX,
                                true,
                                tr("Detected hazard behavior! msg:%1").arg(alertMsgList.join(','))});
}
};  // namespace ToolBox
};  // namespace KS