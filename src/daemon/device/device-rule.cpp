/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#include "src/daemon/device/device-rule.h"
#include <qt5-log-i.h>
#include <QFile>
#include <QMutex>
#include <QTextStream>
#include "config.h"
#include "ksc-marcos.h"

namespace KS
{

DeviceRule *DeviceRule::instance()
{
    static QMutex mutex;
    static QScopedPointer<DeviceRule> pInst;
    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new DeviceRule());
        }
    }
    return pInst.data();
}

DeviceRule::DeviceRule(QObject *parent) : QObject(parent)
{
    this->init();
}

void DeviceRule::init()
{
    QFile file(SC_DEVICE_UDEV_RULES_FILE);

    RETURN_IF_FALSE(file.open(QIODevice::ReadOnly | QIODevice::Text))

    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine();

        m_ruleList << line;
    }

    m_ruleList.removeDuplicates();

    file.close();
}

bool DeviceRule::addRule(const QString &rule)
{
    if (m_ruleList.contains(rule))
    {
        KLOG_WARNING() << "Faile to add rule because have exist: " << rule;
        return false;
    }

    m_ruleList << rule;

    return this->updateRulesToFile();
}

bool DeviceRule::updateRule(const QString &rule, const QString &newRule)
{
    m_ruleList.replaceInStrings(rule, newRule);

    return this->updateRulesToFile();
}

bool DeviceRule::updateRulesToFile()
{
    QFile file(SC_DEVICE_UDEV_RULES_FILE);

    // 触发系统事件，让systemd-udevd服务重新加载规则文件
    QFile::remove(SC_DEVICE_UDEV_RULES_FILE);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        KLOG_ERROR() << "Can not open file: ." << SC_DEVICE_UDEV_RULES_FILE;
        return false;
    }

    QTextStream out(&file);

    auto context = m_ruleList.join("\n");

    out << context << "\n";

    file.close();

    return true;
}

QString DeviceRule::findRule(const QString &str)
{
    auto list = m_ruleList.filter(str);

    RETURN_VAL_IF_TRUE(list.isEmpty(), nullptr)

    return list.first();
}

}  // namespace KS
