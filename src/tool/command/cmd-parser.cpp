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
    for (auto iter = m_brItemInfo.begin(); iter != m_brItemInfo.end(); ++iter)
    {
        delete iter.value();
    }
    m_brItemInfo.clear();
    for (auto iter = m_repairResult.begin(); iter != m_repairResult.end(); ++iter)
    {
        delete iter.value();
    }
    m_repairResult.clear();
}

void Command::checkLicenseActive()
{
    m_licenseProxy = LicenseProxy::getDefault();
    if (!m_licenseProxy->isActivated())
    {
        std::cout << tr("The software is not activated.").toStdString() << std::endl;
        exit(-1);
    }
}

void Command::setFileOutput(bool fileOutput)
{
    m_fileOutput = fileOutput;
}

int Command::brScan()
{
    std::cout << tr("Scannig...").toStdString() << std::endl;
    disconnect(m_dbusBRProxy, &BRDbusProxy::ScanProgress, 0, 0);
    disconnect(m_dbusBRProxy, &BRDbusProxy::ProgressFinished, 0, 0);
    // 进行一次扫描 仅获取扫描结果，不对UI进行调整
    connect(m_dbusBRProxy, &BRDbusProxy::ScanProgress, this, [this](const QString &jobResult)
            {
                ssrJobResult(jobResult);
            });
    connect(m_dbusBRProxy, &BRDbusProxy::ProgressFinished, this, [this]
            {
                disconnect(m_dbusBRProxy, &BRDbusProxy::ScanProgress, 0, 0);
                disconnect(m_dbusBRProxy, &BRDbusProxy::ProgressFinished, 0, 0);
                KLOG_DEBUG() << "ProgressFinished";
                outputBrResult();
            });

    QStringList items = getBrInfo();
    m_dbusBRProxy->Scan(items);
    return 0;
}

int Command::vulnerabilityScan()
{
    std::cout << tr("Scannig...").toStdString() << std::endl;
    m_onlyScan = true;
    m_lastPercent = 0;
    connect(m_dbusVulnerabilityProxy, &VulnerabilityDbusProxy::ScanProgress, this, &Command::scanProgress, Qt::QueuedConnection);
    m_dbusVulnerabilityProxy->Scan();
    return 0;
}

int Command::reinforce(const QStringList &name)
{
    std::cout << tr("Reinforcing...").toStdString() << std::endl;
    connect(m_dbusBRProxy, &BRDbusProxy::ReinforceProgress, this, [this](const QString &jobResult)
            {
                ssrJobResult(jobResult);
            });
    connect(m_dbusBRProxy, &BRDbusProxy::ProgressFinished, this, [this]
            {
                KLOG_INFO() << "ProgressFinished";
                outputBrResult();
            });
    QStringList items = getBrInfo();
    if (!name.isEmpty())
        items = name;
    KLOG_DEBUG() << "reinforce items:" << items;
    auto reply = m_dbusBRProxy->Reinforce(items);
    reply.waitForFinished();
    if (reply.isError())
    {
        KLOG_ERROR() << "error:" << reply.error().message();
        std::cout << tr("Reinforcement Failure").toStdString() << std::endl;
        exit(-1);
    }
    return 0;
}

void Command::repair(const QStringList &cves)
{
    m_onlyScan = false;
    m_lastPercent = 0;
    m_cveIds = cves;
    connect(m_dbusVulnerabilityProxy, &VulnerabilityDbusProxy::RepairProgress, this, &Command::repairProgress);
    if (cves.isEmpty())
    {
        std::cout << tr("Scannig...").toStdString() << std::endl;
        connect(m_dbusVulnerabilityProxy, &VulnerabilityDbusProxy::ScanProgress, this, &Command::scanProgress);
        m_dbusVulnerabilityProxy->Scan();
    }
    else
    {
        getCVEsInfo();
        KLOG_DEBUG() << "CVE Ids:" << m_cveIds;
        std::cout << tr("Repairing...").toStdString() << std::endl;
        m_dbusVulnerabilityProxy->Repair(m_cveIds);
    }
}

int Command::exportReport(QString which, QString path)
{
    KLOG_INFO() << which << "exportPath:" << path;
    if (!path.endsWith(".pdf"))
    {
        std::cout << tr("File name suffix error, please end with .pdf").toStdString() << std::endl;
        exit(-1);
    }
    QFileInfo fileInfo(path);
    QDir dir(fileInfo.absolutePath());
    if (!dir.exists())
    {
        std::cout << tr("The specified directory does not exist").toStdString() << std::endl;
        exit(-1);
    }
    //    auto reply = "br" == which ? m_dbusBRProxy->ExportReport(path) : m_dbusVulnerabilityProxy->ExportReport(path);
    //    reply.waitForFinished();
    //    if (reply.isError())
    //    {
    //        KLOG_WARNING() << "error:" << reply.error().message();
    //        std::cout << tr("Failed to export report").toStdString() << std::endl;
    //        exit(-1);
    //    }

    //    std::cout << tr("Export Report Success").toStdString() << std::endl;
    //    exit(0);

    connect(m_dbusBRProxy, &BRDbusProxy::ExportReportFinished, this, &Command::exportReportFinished, Qt::QueuedConnection);
    connect(m_dbusVulnerabilityProxy, &VulnerabilityDbusProxy::ExportReportFinished, this, &Command::exportReportFinished, Qt::QueuedConnection);

    "br" == which ? m_dbusBRProxy->ExportReport(path) : m_dbusVulnerabilityProxy->ExportReport(path);
    return 0;
}

QStringList Command::getBrInfo(const QStringList &category)
{
    for (auto iter = m_brItemInfo.begin(); iter != m_brItemInfo.end(); ++iter)
    {
        delete iter.value();
    }
    m_brItemInfo.clear();

    QStringList ret;
    auto reply = m_dbusBRProxy->GetReinforcements();
    reply.waitForFinished();
    if (reply.isError() || reply.value() == "")
    {
        KLOG_WARNING() << "error:" << reply.error().message();
        return ret;
    }
    KLOG_DEBUG() << "param category list:" << category;
    const QString xmlString = reply.value();
    QLocale local;
    std::istringstream istringStream(xmlString.toStdString());
    auto rsReinforcements = KS::Protocol::br_reinforcements(istringStream, xml_schema::Flags::dont_validate);
    auto rsReinforcement = rsReinforcements.get()->reinforcement();
    for (auto iter : rsReinforcement)
    {
        QString defaultLabel;
        for (auto label : iter.label())
        {
            if (label.lang() == nullptr)
            {
                defaultLabel = QString(label.c_str());
                continue;
            }

            if (local.name().toStdString() == label.lang().get())
            {
                defaultLabel = QString(label.c_str());
            }
        }
        BrInfo *pBr = new BrInfo(iter.category().get().c_str(), defaultLabel);
        m_brItemInfo[iter.name().c_str()] = pBr;
        if (category.isEmpty() || category.indexOf(iter.category().get().c_str()) == 0)
        {
            ret << iter.name().c_str();
        }
    }
    KLOG_DEBUG() << ret;
    return ret;
}

bool Command::ssrJobResult(const QString &xmlString)
{
    if (xmlString.isEmpty())
        return false;
    std::istringstream istringStream(xmlString.toStdString());
    auto jobResult = KS::Protocol::br_job_result(istringStream, xml_schema::Flags::dont_validate);
    for (auto reinforcement : jobResult->reinforcement())
    {
        if (reinforcement.error() != nullptr)
        {
            KLOG_WARNING() << "error:" << reinforcement.error().get().c_str();
        }
        QString name = reinforcement.name().c_str();
        if (!m_brItemInfo.contains(name))
        {
            BrInfo *pBr = new BrInfo(QString(""), QString(""));
            m_brItemInfo[name] = pBr;
        }
        m_brItemInfo.value(name)->state = state2Str(reinforcement.state());
        m_getBrJob = true;
    }

    return true;
}

int Command::displayWidth(const QString &str)
{
    int width = 0;
    for (const QChar &ch : str)
    {
        if (ch.unicode() < 128)
        {
            width += 1;  // ASCII characters
        }
        else
        {
            width += 2;  // Non-ASCII characters (e.g., Chinese)
        }
    }
    return width;
}

QString Command::leftJustify(const QString &str, int width, QChar fillChar)
{
    int strWidth = displayWidth(str);
    if (strWidth >= width)
    {
        return str;
    }
    return str + QString(width - strWidth, fillChar);
}

void Command::outputBrResult()
{
    if (!m_getBrJob)
        return;

    if (!m_fileOutput)
    {
        for (const auto &key : m_brItemInfo.keys())
        {
            QString label = m_brItemInfo.value(key)->label;
            QString state = m_brItemInfo.value(key)->state;
            if (state.isEmpty())
                continue;
            std::string color = state == QString(tr("Conformity")) || state == QString(tr("Reinforced")) ? "\033[0m" : "\033[31m";
            std::cout << leftJustify(key, 50).toStdString() << leftJustify(label, 50).toStdString() << color << leftJustify(state, 20).toStdString() << "\033[0m" << std::endl;
        }

        exit(0);
    }
    QString timeStr = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss");
    QString fileName = QString(tr("KylinSecHostReinforcementReport_%1_%2_%3.txt")).arg(QSysInfo::machineHostName()).arg(getIPPath()).arg(timeStr);
    QFile f(fileName);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        std::cout << tr("open file failed").toStdString() << std::endl;
        exit(-1);
    }
    QTextStream txtOutput(&f);
    txtOutput.setCodec("UTF-8");  // 确保使用 UTF-8 编码
    for (const auto &key : m_brItemInfo.keys())
    {
        QString label = m_brItemInfo.value(key)->label;
        QString state = m_brItemInfo.value(key)->state;
        if (state.isEmpty())
            continue;
        txtOutput << leftJustify(key, 50) << leftJustify(label, 50) << leftJustify(state, 20) << "\n";
    }
    f.close();
    std::cout << tr("Results output to file ").toStdString() << fileName.toStdString() << std::endl;
    exit(0);
}

void Command::outputRepairResult(QTextStream &output)
{
    auto cveList = m_repairResult.values();
    std::sort(cveList.begin(), cveList.end(), [](const KS::Command::VulnerabilityInfo *a, const KS::Command::VulnerabilityInfo *b)
              {
                  return (a->score).toDouble() > (b->score).toDouble();
              });

    output.setCodec("UTF-8");  // 确保使用 UTF-8 编码
    QMap<QString, int> levelMap;
    QMap<QString, int> stateMap;
    int all = 0;
    for (const auto &cve : cveList)
    {
        if (cve->id.isEmpty())
            continue;
        QString threat_severity = cve->threat_severity;
        QString score = cve->score;
        QString state = m_onlyScan ? "" : cve->state;
        all++;
        levelMap[threat_severity]++;
        stateMap[state]++;
        if (m_fileOutput)
        {
            output << leftJustify(cve->id, 30) << leftJustify(threat_severity, 20) << leftJustify(score, 20) << leftJustify(state, 20) << "\n";
        }
        else
        {
            QString color = state.isEmpty() || state == QString(tr("succeed")) ? "\033[0m" : "\033[31m";
            output << leftJustify(cve->id, 30) << leftJustify(threat_severity, 20) << leftJustify(score, 20) << color << leftJustify(state, 20) << "\033[0m"
                   << "\n";
        }
    }
    output << tr("Total number of vulnerabilities: ") << QString::number(all) << tr(" ");
    for (const auto &key : levelMap.keys())
    {
        output << key << tr(": ") << QString::number(levelMap.value(key)) << tr(" ");
    }
    if (!m_onlyScan)
    {
        for (const auto &key : stateMap.keys())
        {
            output << key << tr(": ") << QString::number(stateMap.value(key)) << tr(" ");
        }
    }

    output << "\n";
}

void Command::outputRepairResult()
{
    if (m_repairResult.size() == 0)
        return;

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
