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

#pragma once

#include <QDBusServiceWatcher>
#include <QMap>
#include <QObject>
#include <QTextStream>

class BRDbusProxy;
class VulnerabilityDbusProxy;
namespace KS
{
class LicenseProxy;
namespace Command
{
enum ModuleType
{
    MODULE_BR = 1,
    MODULE_VULNERABILITY,
};

struct OutputInfo
{
    OutputInfo(QString _name)
        : name(_name) {}
    QString name;
    QString secondColumn;  // for vulnerability: threat_severity; for br: label
    QString thirdColumn;   // for vulnerability: score
    QString state;
};

class Command : public QObject
{
    Q_OBJECT
public:
    explicit Command(QObject *parent = nullptr);
    virtual ~Command();
    void setFileOutput(bool fileOutput);
    int brScan();
    int brReinforce(const QStringList &name = QStringList());
    int brExport(const QString &filePath);
    int vulnerabilityScan();
    int vulnerabilityRepair(const QStringList &name = QStringList());
    int vulnerabilityExport(const QString &filePath);

private:
    void checkLicenseActive();
    void addDbusServerWatcher();
    void moduleBrInit();
    void moduleVulnerabilityInit();
    void brOutputResult(QTextStream &output);
    void vulnerabilityOutputResult(QTextStream &output);
    void outputResult(QTextStream &output, ModuleType type);
    void outputMethodProcess(ModuleType type);
    int checkExportPath(const QString &filePath);
    int getCVEsInfo(const QStringList &name);
    int brJobResultProcess(const QString &xmlString);
    QStringList getReinforcements(const QStringList &specifyList = QStringList());
    QString leftJustify(const QString &str, int width, QChar fillChar = ' ');
    QString getCveLevel(int level);
    QString getCveState(int state);
    QString state2Str(int state);
    QJsonObject str2jsonObject(const QString &str);

private slots:
    void scanProgress(const QString &progress);
    void repairProgress(const QString &progress);
    void exportReportFinished(const QString &failed_reason);

private:
    BRDbusProxy *m_dbusBRProxy;
    VulnerabilityDbusProxy *m_dbusVulnerabilityProxy;
    QDBusServiceWatcher *m_dbusServerWatcher;
    QSharedPointer<LicenseProxy> m_licenseProxy;
    bool m_fileOutput;
    bool m_getBrJob;
    bool m_onlyScan;
    int m_lastPercent;
    QStringList m_cveIds;
    QMap<QString, BrInfo *> m_brItemInfo;
    QMap<QString, VulnerabilityInfo *> m_repairResult;
    QStringList m_notExistCVE;
};
}  // namespace Command
}  // namespace KS
