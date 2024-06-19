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
#include <QSharedPointer>
#include <QTextStream>
#include "lib/dbus/license-proxy.h"

class BRDbusProxy;
class VulnerabilityDbusProxy;
namespace KS
{
class LicenseProxy;
namespace Command
{
struct VulnerabilityInfo
{
    VulnerabilityInfo(QString _id, QString _threat_severity, QString _score)
        : id(_id), threat_severity(_threat_severity), score(_score) {}
    QString id;
    QString threat_severity;
    QString score;
    QString state;
};

struct BrInfo
{
    BrInfo(QString _category, QString _label)
        : category(_category), label(_label) {}
    QString category;
    QString label;
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
    int vulnerabilityScan();
    int reinforce(const QStringList &name = QStringList());
    void repair(const QStringList &name = QStringList());
    int exportReport(QString which, QString path);

private:
    void checkLicenseActive();
    QStringList getBrInfo(const QStringList &category = QStringList());
    bool ssrJobResult(const QString &xmlString);
    int displayWidth(const QString &str);
    QString leftJustify(const QString &str, int width, QChar fillChar = ' ');
    void outputBrResult(QString fileName);
    void outputRepairResult(QTextStream &output);
    void outputRepairResult(QString fileName);
    QString getCveLevel(int level);
    QString getCveState(int state);
    QString state2Str(int state);
    QString python2Translate(const QString &souceTxt);
    QString noop2Translate(const QString &souceTxt);
    QString categoriesLabel2Translate(const QString &souceTxt);
    QJsonObject str2jsonObject(const QString &str);
    int getCVEsInfo();

private slots:
    void scanProgress(const QString &progress);
    void repairProgress(const QString &progress);
    void exportReportFinished(const QString &failed_reason);

private:
    BRDbusProxy *m_dbusBRProxy;
    VulnerabilityDbusProxy *m_dbusVulnerabilityProxy;
    QSharedPointer<LicenseProxy> m_licenseProxy;
    bool m_fileOutput;
    bool m_getBrJob;
    bool m_onlyScan;
    int m_lastPercent;
    QStringList m_cveIds;
    QMap<QString, BrInfo *> m_brItemInfo;
    QMap<QString, VulnerabilityInfo *> m_repairResult;
};
}  // namespace Command
}  // namespace KS
