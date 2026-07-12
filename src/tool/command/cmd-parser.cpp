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

int Command::backup(QString directory)
{
    moduleVulnerabilityInit();
    if (!QFileInfo::exists(directory))
    {
        std::cout << tr("The backup directory does not exist").toStdString() << std::endl;
        exit(-1);
    }

    std::cout << tr("Start backup").toStdString() << std::endl;
    connect(m_dbusVulnerabilityProxy, &VulnerabilityDbusProxy::BackUpProgress, this, &Command::backupProgress);
    auto reply = m_dbusVulnerabilityProxy->BackUp(directory);
    reply.waitForFinished();
    if (reply.isError())
    {
        disconnect(m_dbusVulnerabilityProxy, &VulnerabilityDbusProxy::BackUpProgress, nullptr, nullptr);
        std::cout << tr("BackUp Failure, error message: ").toStdString() << reply.error().message().toStdString() << std::endl;
        exit(-1);
    }

    return 0;
}

int Command::rollback()
{
    moduleVulnerabilityInit();
    auto reply = m_dbusVulnerabilityProxy->GetBackUpInfo();
    reply.waitForFinished();
    if (reply.isError())
    {
        std::cout << tr("Check backup Failure, error message: ").toStdString() << reply.error().message().toStdString() << std::endl;
        exit(-1);
    }

    QJsonObject backupJson = StrUtils::str2jsonObject(reply);
    QString backupPath = backupJson.value("path").toString();
    int backupSize = backupJson.value("size").toInt();

    if (backupJson.isEmpty() || backupPath.isEmpty() || 0 == backupSize)
    {
        std::cout << tr("Backup data not detected").toStdString() << std::endl;
        exit(-1);
    }

    std::cout << tr("Existing backup data:\n").toStdString()
              << tr("path:").toStdString() << backupPath.toStdString() << "\t" << tr("size:").toStdString() << backupSize << std::endl;

    std::cout << tr("Start rollback").toStdString() << std::endl;
    connect(m_dbusVulnerabilityProxy, &VulnerabilityDbusProxy::RollBackProgress, this, &Command::rollBackProgress);
    reply = m_dbusVulnerabilityProxy->RollBack();
    reply.waitForFinished();
    if (reply.isError())
    {
        disconnect(m_dbusVulnerabilityProxy, &VulnerabilityDbusProxy::RollBackProgress, nullptr, nullptr);
        std::cout << tr("RollBack Failure, error message: ").toStdString() << reply.error().message().toStdString() << std::endl;
        exit(-1);
    }

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
            connect(m_dbusServerWatcher, &QDBusServiceWatcher::serviceUnregistered, [](const QString &service)
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
    if (!m_dbusBRProxy)
    {
        m_dbusBRProxy = new BRDbusProxy(SSR_DBUS_NAME,
                                        BR_DBUS_OBJECT_PATH,
                                        QDBusConnection::systemBus(),
                                        this);
    }
}

void Command::moduleVulnerabilityInit()
{
    if (!m_dbusVulnerabilityProxy)
    {
        m_dbusVulnerabilityProxy = new VulnerabilityDbusProxy(SSR_DBUS_NAME,
                                                              SSR_VULNERABILITY_DBUS_OBJECT_PATH,
                                                              QDBusConnection::systemBus(),
                                                              this);
    }
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
        QString timeStr = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
        QString fileName = tr("KylinSecHostReinforcementReport_%1_%2_%3.txt").arg(QSysInfo::machineHostName()).arg(getIPPath()).arg(timeStr);
        QFile f(fileName);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            std::cout << tr("open file failed").toStdString() << std::endl;
            exit(-1);
        }
        QTextStream txtOutput(&f);
        outputResult(txtOutput, type);
        f.close();
        std::cout << tr("Results output to file ").toStdString() << fileName.toStdString() << std::endl;
    }
    else
    {
        QTextStream txtOutput(stdout);
        outputResult(txtOutput, type);
    }

    exit(0);
}

int Command::checkExportPath(const QString &filePath)
{
    KLOG_INFO() << "exportPath:" << filePath;
    if (!filePath.endsWith(".pdf"))
    {
        std::cout << tr("File name suffix error, please end with .pdf").toStdString() << std::endl;
        return -1;
    }
    QFileInfo fileInfo(filePath);
    QDir dir(fileInfo.absolutePath());
    if (!dir.exists())
    {
        std::cout << tr("The specified directory does not exist").toStdString() << std::endl;
        return -1;
    }

    return 0;
}

int Command::getCVEsInfo(const QStringList &name)
{
    auto reply = m_dbusVulnerabilityProxy->GetCVEsInfo(name);
    reply.waitForFinished();
    if (reply.isError())
    {
        std::cout << tr("Failed to get CVE information, error message: ").toStdString() << reply.error().message().toStdString() << std::endl;
        return -1;
    }

    QJsonDocument document = QJsonDocument::fromJson(reply.value().toLocal8Bit());
    if (!document.isArray())
    {
        std::cout << tr("The return data is not a JSON array").toStdString() << std::endl;
        return -1;
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
        QString objName = jsonObject["name"].toString();
        if (objName.isEmpty())
        {
            continue;
        }
        OutputInfo *pInfo = new OutputInfo(objName);
        pInfo->secondColumn = getCveLevel(jsonObject["threat_severity"].toInt());
        pInfo->thirdColumn = jsonObject["score"].toString();
        m_outputInfo[objName] = pInfo;
    }
    KLOG_INFO() << m_outputInfo.keys();
    return 0;
}

int Command::brJobResultProcess(const QString &xmlString)
{
    if (xmlString.isEmpty())
        return -1;

    std::istringstream istringStream(xmlString.toStdString());
    auto jobResult = KS::Protocol::br_job_result(istringStream, xml_schema::Flags::dont_validate);
    for (auto reinforcement : jobResult->reinforcement())
    {
        if (reinforcement.error())
        {
            KLOG_WARNING() << "error:" << reinforcement.error().get().c_str();
        }
        QString name = reinforcement.name().c_str();
        if (!m_outputInfo.contains(name))
        {
            OutputInfo *pInfo = new OutputInfo(name);
            m_outputInfo[name] = pInfo;
        }
        m_outputInfo.value(name)->state = state2Str(reinforcement.state());
    }

    return 0;
}

QStringList Command::getReinforcements(const QStringList &specifyList)
{
    QStringList ret;
    auto reply = m_dbusBRProxy->GetReinforcements();
    reply.waitForFinished();
    if (reply.isError() || reply.value() == "")
    {
        KLOG_WARNING() << "error:" << reply.error().message();
        return ret;
    }

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
            if (!label.lang())
            {
                defaultLabel = QString(label.c_str());
                continue;
            }

            if (local.name().toStdString() == label.lang().get())
            {
                defaultLabel = QString(label.c_str());
            }
        }

        QString name = iter.name().c_str();
        if (!specifyList.isEmpty() && specifyList.indexOf(name) == -1)
        {
            continue;
        }
        OutputInfo *pInfo = new OutputInfo(name);
        pInfo->secondColumn = defaultLabel;
        m_outputInfo[name] = pInfo;
        ret << name;
    }
    KLOG_INFO() << ret;
    return ret;
}

QString Command::leftJustify(const QString &str, int width, QChar fillChar)
{
    int strWidth = 0;
    for (const QChar &ch : str)
    {
        if (ch.unicode() < 128)
        {
            strWidth += 1;
        }
        else
        {
            strWidth += 2;
        }
    }
    if (strWidth >= width)
    {
        return str;
    }

    return str + QString(width - strWidth, fillChar);
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

        auto name = cve.value("name").toString();
        OutputInfo *pInfo = new OutputInfo(name);
        pInfo->secondColumn = getCveLevel(cve.value("threat_severity").toInt());
        pInfo->thirdColumn = cve.value("score").toString();
        m_outputInfo[name] = pInfo;
    }

    int percent = progressJson.value("progress").toInt();
    if (m_lastPercent != percent)
    {
        m_lastPercent = percent;
        std::cout << tr("Scan progress ").toStdString() << std::to_string(percent) << std::endl;
    }
    if (100 == percent)
    {
        if (m_outputInfo.isEmpty())
        {
            std::cout << tr("No system vulnerabilities were found in this scan").toStdString() << std::endl;
            exit(0);
        }

        if (m_onlyScan)
        {
            outputMethodProcess(MODULE_VULNERABILITY);
        }
        else
        {
            std::cout << tr("Repairing...").toStdString() << std::endl;
            KLOG_INFO() << QDateTime::currentDateTime().toString("yyyy-MM-dd_hh:mm:ss");
            m_lastPercent = 0;
            auto reply = m_dbusVulnerabilityProxy->Repair(m_outputInfo.keys());
            reply.waitForFinished();
            if (reply.isError())
            {
                std::cout << tr("Repair Failure, error message: ").toStdString() << reply.error().message().toStdString() << std::endl;
                exit(-1);
            }
        }
    }
}

void Command::repairProgress(const QString &progress)
{
    QJsonObject progressJson = str2jsonObject(progress);
    int percent = progressJson.value("progress").toInt();
    auto errorMessage = progressJson.value("errorMessage").toVariant().toString();
    if (-1 == percent)
    {
        std::cout << tr("Repair stop by manually cancel!").toStdString();
        exit(0);
    }
    if (!errorMessage.isEmpty())
    {
        std::cout << tr("error: ").toStdString() << errorMessage.toStdString() << std::endl;
        exit(0);
    }
    if (m_lastPercent != percent)
    {
        m_lastPercent = percent;
        std::cout << tr("Repair progress ").toStdString() << std::to_string(percent) << std::endl;
    }
    if (100 == percent)
    {
        outputMethodProcess(MODULE_VULNERABILITY);
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
