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
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#include "plugin-bash.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>

namespace KS
{
namespace BR
{
#define REINFORCEMENTS_CONFIG_NAME "reinforcements.json"

ReinforcementBash::ReinforcementBash(const QString &bashFilePath)
    : m_bashFilePath(bashFilePath)
{
}

bool ReinforcementBash::get(QString &args, QString &error)
{
    auto retval = QProcess::execute(m_bashFilePath, QStringList{"get"});
    return retval == 0;
}

bool ReinforcementBash::set(const QString &args, QString &error)
{
    auto retval = QProcess::execute(m_bashFilePath, QStringList{"set"});
    return retval == 0;
}

bool ReinforcementBash::backup(QString &args, QString &error)
{
    auto retval = QProcess::execute(m_bashFilePath, QStringList{"backup"});
    return retval == 0;
}

bool ReinforcementBash::rollback(const QString &args, QString &error)
{
    auto retval = QProcess::execute(m_bashFilePath, QStringList{"rollback"});
    return retval == 0;
}

PluginBash::PluginBash(const QString &bashRootDir)
    : m_bashRootDir(bashRootDir)
{
    m_reinforcementConfigPath = QDir::cleanPath(QString("%1/%2").arg(bashRootDir).arg(REINFORCEMENTS_CONFIG_NAME));
}

void PluginBash::activate()
{
    QFile file(m_reinforcementConfigPath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KLOG_ERROR() << "Not to open reinforcement config file" << m_reinforcementConfigPath;
        return;
    }

    auto fileContent = file.readAll();
    auto jsonDocument = QJsonDocument::fromJson(fileContent);
    auto jsonReinforcements = jsonDocument.array();

    for (const auto &iter : jsonReinforcements)
    {
        auto jsonReinforcement = iter.toObject();
        auto name = jsonReinforcement.value("name").toString();
        auto scriptName = jsonReinforcement.value("script").toString();
        auto scriptPath = QDir::cleanPath(QString("%1/%2").arg(m_bashRootDir).arg(scriptName));
        auto reinforcement = QSharedPointer<ReinforcementBash>(new ReinforcementBash(scriptPath));

        if (this->m_reinforcements.find(name) != this->m_reinforcements.end())
        {
            KLOG_WARNING() << "The reinforcement " << name << " is repeated.";
            continue;
        }
        else
        {
            this->m_reinforcements[name] = reinforcement;
        }
    }

    KLOG_INFO() << "Bash plugin" << m_bashRootDir << "is activated. The plugin contains reinforcements" << m_reinforcements.keys();
}

void PluginBash::deactivate()
{
    this->m_reinforcements.clear();
}
}  // namespace BR
}  // namespace KS
