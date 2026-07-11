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
 * Author:     zhenggongping <zhenggongping@kylinos.com.cn>
 */
#include "cmd-parser.h"
#include <unistd.h>
#include <QJsonObject>
#include <QSharedPointer>
#include <QTextCodec>
#include <iostream>
#include "br-protocol.hxx"
#include "br_dbus_proxy.h"
#include "include/ssr-i.h"
#include "lib/base/str-utils.h"
#include "lib/base/sys-info.h"
#include "lib/dbus/license-proxy.h"
#include "qt5-log-i.h"
#include "vulnerability_dbus_proxy.h"

namespace KS
{
namespace Command
{
Command::Command(QObject *parent)
    : QObject(parent),
      m_fileOutput(false),
      m_onlyScan(true),
      m_lastPercent(0),
      m_dbusServerWatcher(new QDBusServiceWatcher(this)),
      m_dbusBRProxy(nullptr),
      m_dbusVulnerabilityProxy(nullptr)
{
    checkLicenseActive();
    addDbusServerWatcher();
}

Command::~Command()
{
    if (m_dbusBRProxy)
    {
        delete m_dbusBRProxy;
        m_dbusBRProxy = nullptr;
    }
    if (m_dbusVulnerabilityProxy)
    {
        delete m_dbusVulnerabilityProxy;
        m_dbusVulnerabilityProxy = nullptr;
    }
    if (m_dbusServerWatcher)
    {
        delete m_dbusServerWatcher;
        m_dbusServerWatcher = nullptr;
    }
    for (auto iter = m_outputInfo.begin(); iter != m_outputInfo.end(); ++iter)
    {
        delete iter.value();
    }
    m_outputInfo.clear();
}

void Command::setFileOutput(bool fileOutput)
{
    m_fileOutput = fileOutput;
}

int Command::brScan()
{
    moduleBrInit();
    std::cout << tr("Scannig...").toStdString() << std::endl;
    connect(m_dbusBRProxy, &BRDbusProxy::ScanProgress, this, [this](const QString &jobResult)
            {
                brJobResultProcess(jobResult);
            });
    connect(m_dbusBRProxy, &BRDbusProxy::ProgressFinished, this, [this]
            {
                disconnect(m_dbusBRProxy, &BRDbusProxy::ScanProgress, 0, 0);
                disconnect(m_dbusBRProxy, &BRDbusProxy::ProgressFinished, 0, 0);
                outputMethodProcess(MODULE_BR);
            });

    m_dbusBRProxy->Scan(getReinforcements());
    return 0;
}

int Command::brReinforce(const QStringList &name)
{
    moduleBrInit();
    std::cout << tr("Reinforcing...").toStdString() << std::endl;
    connect(m_dbusBRProxy, &BRDbusProxy::ReinforceProgress, this, [this](const QString &jobResult)
            {
                brJobResultProcess(jobResult);
            });
    connect(m_dbusBRProxy, &BRDbusProxy::ProgressFinished, this, [this]
            {
                KLOG_INFO() << "ProgressFinished";
                outputMethodProcess(MODULE_BR);
            });
    QStringList items = getReinforcements(name);
    if (items.isEmpty())
    {
        std::cout << tr("Failed to get reinforcements").toStdString() << std::endl;
        return -1;
    }
    auto reply = m_dbusBRProxy->Reinforce(items);
    reply.waitForFinished();
    if (reply.isError())
    {
        std::cout << tr("Reinforcement failed, error message: ").toStdString() << reply.error().message().toStdString() << std::endl;
        return -1;
    }
    return 0;
}
int Command::brExport(const QString &filePath)
{
    if (filePath.isEmpty())
    {
        std::cout << tr("Please enter the pdf file name").toStdString() << std::endl;
        return -1;
    }
    QString path = QDir(QDir::currentPath()).absoluteFilePath(filePath);
    if (checkExportPath(path) != 0)
    {
        return -1;
    }
    moduleBrInit();
    connect(m_dbusBRProxy, &BRDbusProxy::ExportReportFinished, this, &Command::exportReportFinished);
    m_dbusBRProxy->ExportReport(path);
    return 0;
}

int Command::vulnerabilityScan()
{
    moduleVulnerabilityInit();
    std::cout << tr("Scannig...").toStdString() << std::endl;
    connect(m_dbusVulnerabilityProxy, &VulnerabilityDbusProxy::ScanProgress, this, &Command::scanProgress);
    m_dbusVulnerabilityProxy->Scan();
    return 0;
}

int Command::vulnerabilityRepair(const QStringList &name)
{
    moduleVulnerabilityInit();
    connect(m_dbusVulnerabilityProxy, &VulnerabilityDbusProxy::RepairProgress, this, &Command::repairProgress);
    if (name.isEmpty())
    {
        std::cout << tr("Scannig...").toStdString() << std::endl;
        m_onlyScan = false;
        connect(m_dbusVulnerabilityProxy, &VulnerabilityDbusProxy::ScanProgress, this, &Command::scanProgress);
        m_dbusVulnerabilityProxy->Scan();
    }
    else
    {
        if (0 != getCVEsInfo(name))
        {
            return -1;
        }
        KLOG_DEBUG() << "CVE Ids:" << name;
#if 0
        auto notExist = name.toSet().subtract(m_outputInfo.keys().toSet());
        std::string cveStr = "\"";
        for (const auto &cve : notExist)
        {
            cveStr = cveStr + cve.toStdString() + ",";
        }
        cveStr = cveStr.substr(0, cveStr.size() - 1) + "\"";
        if (!notExist.isEmpty())
        {
            std::cout << tr("Vulnerability ").toStdString() << cveStr << tr(" does not exist").toStdString() << std::endl;
        }
        if (m_outputInfo.isEmpty())
        {
            return -1;
        }
#endif
        std::cout << tr("Repairing...").toStdString() << std::endl;
        auto reply = m_dbusVulnerabilityProxy->Repair(m_outputInfo.keys());
        reply.waitForFinished();
        if (reply.isError())
        {
            std::cout << tr("Repair Failure, error message: ").toStdString() << reply.error().message().toStdString() << std::endl;
            return -1;
        }
    }
    return 0;
}

int Command::vulnerabilityExport(const QString &filePath)
{
    QString path = QDir(QDir::currentPath()).absoluteFilePath(filePath);
    if (checkExportPath(path) != 0)
    {
        return -1;
    }

    moduleVulnerabilityInit();
    connect(m_dbusVulnerabilityProxy, &VulnerabilityDbusProxy::ExportReportFinished, this, &Command::exportReportFinished);
    m_dbusVulnerabilityProxy->ExportReport(path);
    return 0;
}

void Command::checkLicenseActive()
{
    QSharedPointer<LicenseProxy> licenseProxy = LicenseProxy::getDefault();
    if (!licenseProxy->isActivated())
    {
        std::cout << tr("The software is not activated.").toStdString() << std::endl;
        exit(-1);
    }
}

void Command::addDbusServerWatcher()
{
    QDBusConnection connection = QDBusConnection::systemBus();
    QDBusConnectionInterface *interface = connection.interface();
    if (interface)
    {
        QDBusReply<QString> reply = interface->serviceOwner(SSR_DBUS_NAME);
        if (reply.isValid())
        {
            KLOG_INFO() << "Service UniqueName:" << reply.value();
            m_dbusServerWatcher->setConnection(connection);
            m_dbusServerWatcher->setWatchMode(QDBusServiceWatcher::WatchForOwnerChange);
            m_dbusServerWatcher->addWatchedService(reply.value());
            connect(m_dbusServerWatcher, &QDBusServiceWatcher::serviceUnregistered, [this](const QString &service)
                    {
                        std::cout << tr("The background daemon service exits. The unique name of the dbus service: ").toStdString() << service.toStdString() << std::endl;
                        exit(-1);
                    });
        }
        else
        {
            KLOG_ERROR() << "Failed to get the UniqueName for service:" << SSR_DBUS_NAME;
        }
    }
    else
    {
        KLOG_ERROR() << "Failed to get the DBus connection interface.";
    }
}

void Command::moduleBrInit()
{
    m_dbusBRProxy = new BRDbusProxy(SSR_DBUS_NAME,
                                    BR_DBUS_OBJECT_PATH,
                                    QDBusConnection::systemBus(),
                                    this);
}

void Command::moduleVulnerabilityInit()
{
    m_dbusVulnerabilityProxy = new VulnerabilityDbusProxy(SSR_DBUS_NAME,
                                                          SSR_VULNERABILITY_DBUS_OBJECT_PATH,
                                                          QDBusConnection::systemBus(),
                                                          this);
}

void Command::brOutputResult(QTextStream &output)
{
    output.setCodec("UTF-8");
    for (const auto &key : m_outputInfo.keys())
    {
        QString label = m_outputInfo.value(key)->secondColumn;
        QString state = m_outputInfo.value(key)->state;
        output << leftJustify(key, 50) << leftJustify(label, 50);
        if (m_fileOutput)
        {
            output << leftJustify(state, 20) << "\n";
        }
        else
        {
            QString color = state.isEmpty() || QString(tr("Conformity")) == state || QString(tr("Reinforced")) == state ? "\033[0m" : "\033[31m";
            output << color << leftJustify(state, 20) << "\033[0m"
                   << "\n";
        }
    }
}

void Command::vulnerabilityOutputResult(QTextStream &output)
{
    auto cveList = m_outputInfo.values();
    std::sort(cveList.begin(), cveList.end(), [](const KS::Command::OutputInfo *a, const KS::Command::OutputInfo *b)
              {
                  return (a->thirdColumn).toDouble() > (b->thirdColumn).toDouble();
              });

    output.setCodec("UTF-8");  // 确保使用 UTF-8 编码
    QMap<QString, int> levelMap;
    QMap<QString, int> stateMap;
    for (const auto &cve : cveList)
    {
        QString threat_severity = cve->secondColumn;
        QString score = cve->thirdColumn;
        QString state = cve->state;
        levelMap[threat_severity]++;
        stateMap[state]++;
        output << leftJustify(cve->name, 30) << leftJustify(threat_severity, 20) << leftJustify(score, 20);
        if (m_fileOutput)
        {
            output << leftJustify(state, 20) << "\n";
        }
        else
        {
            QString color = state.isEmpty() || QString(tr("succeed")) == state ? "\033[0m" : "\033[31m";
            output << color << leftJustify(state, 20) << "\033[0m"
                   << "\n";
        }
    }
    output << tr("Total number of vulnerabilities: ") << QString::number(cveList.size()) << tr(" ");
    for (const auto &key : levelMap.keys())
    {
        output << key << tr(": ") << QString::number(levelMap.value(key)) << tr(" ");
    }
    output << "\n";
}

void Command::outputResult(QTextStream &output, ModuleType type)
{
    switch (type)
    {
    case MODULE_BR:
        brOutputResult(output);
        break;
    case MODULE_VULNERABILITY:
        vulnerabilityOutputResult(output);
        break;
    default:
        KLOG_WARNING() << type;
        break;
    }
}

void Command::outputMethodProcess(ModuleType type)
{
    if (m_outputInfo.size() == 0)
    {
        std::cout << tr("Job done").toStdString() << std::endl;
        exit(0);
    }

    QTextStream text;
    if (m_fileOutput)
    {
        QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss");
        QString fileName = QString(tr("KylinSecHostReinforcementReport_%1_%2_%3.txt")).arg(QSysInfo::machineHostName()).arg(getIPPath()).arg(timeStr);
        QFile f(fileName);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            std::cout << tr("open file failed").toStdString() << std::endl;
            exit(-1);
        }
        QTextStream txtOutput(&f);
        outputRepairResult(txtOutput);
        f.close();
        std::cout << tr("Results output to file ").toStdString() << fileName.toStdString() << std::endl;
    }
    else
    {
        QTextStream txtOutput(stdout);
        outputRepairResult(txtOutput);
    }
}

QString Command::getCveLevel(int level)
{
    switch (level)
    {
    case 0:
        return tr("fatal");
    case 1:
        return tr("high");
    case 2:
        return tr("middle");
    default:
        return tr("low");
    }

    return tr("low");
}

QString Command::getCveState(int state)
{
    switch (state)
    {
    case 1:
        return tr("succeed");
    case 2:
        return tr("failed");
    default:
        return tr("not repair");
    }

    return tr("not repair");
}

QString Command::state2Str(int state)
{
    QString retStr;
    switch (state)
    {
    case BR_REINFORCEMENT_STATE_UNKNOWN:
        retStr = QString(tr("Unknown"));
        break;
    case BR_REINFORCEMENT_STATE_SAFE:
        retStr = QString(tr("Conformity"));
        break;
    case BR_REINFORCEMENT_STATE_UNSAFE:
        retStr = QString(tr("Inconformity"));
        break;
    case BR_REINFORCEMENT_STATE_UNSCAN:
        retStr = QString(tr("Not Scanned"));
        break;
    case BR_REINFORCEMENT_STATE_SCANNING:
        retStr = QString(tr("Scannig..."));
        break;
    case BR_REINFORCEMENT_STATE_SCAN_ERROR:
        retStr = QString(tr("Scan Failed"));
        break;
    case BR_REINFORCEMENT_STATE_SCAN_DONE:
        retStr = QString(tr("Scan Complete"));
        break;
    case BR_REINFORCEMENT_STATE_UNREINFORCE:
        retStr = QString(tr("Unreinforcement"));
        break;
    case BR_REINFORCEMENT_STATE_REINFORCING:
        retStr = QString(tr("Reinforcing..."));
        break;
    case BR_REINFORCEMENT_STATE_REINFORCE_ERROR:
        retStr = QString(tr("Reinforcement Failure"));
        break;
    case BR_REINFORCEMENT_STATE_REINFORCE_DONE:
        retStr = QString(tr("Reinforced"));
        break;
    default:
        if ((state & BR_REINFORCEMENT_STATE_SAFE) == 1)
            retStr = QString(tr("Conformity"));
        else if ((state & BR_REINFORCEMENT_STATE_UNSAFE) == 2)
            retStr = QString(tr("Inconformity"));
        else
            retStr = QString(tr("Unknown"));
        break;
    }
    return retStr;
}

QJsonObject Command::str2jsonObject(const QString &str)
{
    auto doc = QJsonDocument::fromJson(str.toLocal8Bit());
    if (doc.isNull() && !doc.isObject())
    {
        KLOG_WARNING() << "Failed to deserialize str: " << str;
    }
    return doc.object();
}

int Command::getCVEsInfo()
{
    auto reply = m_dbusVulnerabilityProxy->GetCVEsInfo(m_cveIds);
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "error:" << reply.error().message();
        std::cout << tr("Failed to get CVE information").toStdString() << std::endl;
        exit(-1);
    }

    QJsonDocument document = QJsonDocument::fromJson(reply.value().toLocal8Bit());
    if (!document.isArray())
    {
        std::cout << tr("The return data is not a JSON array").toStdString() << std::endl;
        exit(-1);
    }

    QJsonArray jsonArray = document.array();
    for (const QJsonValue &value : jsonArray)
    {
        if (!value.isObject())
        {
            KLOG_DEBUG() << "JSON array item is not an object";
            continue;
        }

        QJsonObject jsonObject = value.toObject();
        QString level = getCveLevel(jsonObject["threat_severity"].toInt());
        VulnerabilityInfo *pVu = new VulnerabilityInfo(jsonObject["name"].toString(), level, jsonObject["score"].toString());
        m_repairResult[jsonObject["name"].toString()] = pVu;
    }

    return 0;
}

void Command::scanProgress(const QString &progress)
{
    QJsonObject progressJson = str2jsonObject(progress);
    if (!progressJson.contains("progress") || !progressJson.contains("cveInfos"))
    {
        return;
    }

    QJsonArray cveArray = progressJson.value("cveInfos").toArray();
    for (auto item : cveArray)
    {
        auto cve = item.toObject();
        if (!cve.contains("name"))
        {
            continue;
        }

        m_cveIds << QString(cve.value("name").toString());
        QString level = getCveLevel(cve.value("threat_severity").toInt());
        VulnerabilityInfo *pVu = new VulnerabilityInfo(cve.value("name").toString(), level, cve.value("score").toString());
        m_repairResult[cve.value("name").toString()] = pVu;
    }

    int percent = progressJson.value("progress").toInt();
    if (m_lastPercent != percent)
    {
        m_lastPercent = percent;
        std::cout << tr("Scan progress ").toStdString() << std::to_string(percent) << std::endl;
    }
    if (100 == percent)
    {
        //        QSet<QString> uniqueItems = QSet<QString>(m_cveIds.begin(), m_cveIds.end());
        QSet<QString> uniqueItems = QSet<QString>::fromList(m_cveIds);
        m_cveIds = uniqueItems.values();

        if (m_cveIds.isEmpty())
        {
            std::cout << tr("No system vulnerabilities were found in this scan").toStdString() << std::endl;
            exit(0);
        }

        if (m_onlyScan)
        {
            outputRepairResult();
            exit(0);
        }

        KLOG_DEBUG() << "CVE Ids:" << m_cveIds;
        std::cout << tr("Repairing...").toStdString() << std::endl;
        m_lastPercent = 0;
        m_dbusVulnerabilityProxy->Repair(m_cveIds);
    }
}

void Command::repairProgress(const QString &progress)
{
    QJsonObject progressJson = str2jsonObject(progress);
    QJsonArray cveArray = progressJson.value("RepairInfo").toArray();
    // for (auto item : cveArray)
    // {
    //     auto cve = item.toObject();
    //     if (!cve.contains("cveId"))
    //     {
    //         continue;
    //     }

    //     int state = QString(cve.value("state").toString()).compare("Success", Qt::CaseInsensitive) ? 2 : 1;
    //     QString id = cve.value("cveId").toString();
    //     if (!m_repairResult.contains(id))
    //     {
    //         m_notExistCVE << id;
    //         continue;
    //     }
    //     m_repairResult.value(id)->state = getCveState(state);
    // }

    // 进度
    int percent = progressJson.value("progress").toInt();
    auto errorMessage = progressJson.value("errorMessage").toVariant().toString();
    if (-1 == percent)
    {
        std::cout << tr("Repair stop by manually cancel!").toStdString();
        exit(0);
    }
    if (m_lastPercent != percent)
    {
        m_lastPercent = percent;
        std::cout << tr("Repair progress ").toStdString() << std::to_string(percent) << std::endl;
    }
    if (100 == percent)
    {
        if (!errorMessage.isEmpty())
        {
            std::cout << tr("error: ").toStdString() << errorMessage.toStdString() << std::endl;
            exit(0);
        }
        outputRepairResult();
        // std::string cveStr = "\"";
        // for (const auto &cve : m_notExistCVE)
        // {
        //     cveStr = cveStr + cve.toStdString() + ",";
        // }
        // cveStr = cveStr.substr(0, cveStr.size() - 1) + "\"";
        // if (!m_notExistCVE.isEmpty())
        // {
        //     std::cout << tr("Vulnerability ").toStdString() << cveStr << tr(" does not exist").toStdString() << std::endl;
        // }
        exit(0);
    }
}

void Command::exportReportFinished(const QString &failed_reason)
{
    if (failed_reason.isEmpty())
    {
        std::cout << tr("Export Report Success").toStdString() << std::endl;
    }
    else
    {
        std::cout << tr("Export Report failed:").toStdString() << failed_reason.toStdString() << std::endl;
    }
    exit(0);
}

}  // namespace Command
}  // namespace KS
