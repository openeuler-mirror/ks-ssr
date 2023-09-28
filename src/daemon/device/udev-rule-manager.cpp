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

#include "src/daemon/device/udev-rule-manager.h"
#include <qt5-log-i.h>
#include <QFile>
#include <QTextStream>
#include "config.h"
#include "ksc-i.h"
#include "ksc-marcos.h"

#define PER_BIN_VALUE_READ 4     // 2的2次方
#define PER_BIN_VALUE_WRITE 2    // 2的1次方
#define PER_BIN_VALUE_EXECUTE 1  // 2的0次方

namespace KS
{
UdevRuleManager *UdevRuleManager::instance()
{
    static QScopedPointer<UdevRuleManager> pInst;
    if (Q_UNLIKELY(!pInst))
    {
        if (pInst.isNull())
        {
            pInst.reset(new UdevRuleManager());
        }
    }
    return pInst.data();
}

UdevRuleManager::UdevRuleManager(QObject *parent) : QObject(parent)
{
    this->init();
}

void UdevRuleManager::handleDevConfigChanged()
{
    this->updateUdevRules();
}

void UdevRuleManager::init()
{
    m_deviceConfig = DeviceConfiguration::instance();
    connect(m_deviceConfig, &DeviceConfiguration::deviceConfigChanged, this, &UdevRuleManager::handleDevConfigChanged);

    this->updateUdevRules();
}

void UdevRuleManager::updateUdevRules()
{
    //清除所有项
    m_rules.clear();

    for (auto config : m_deviceConfig->getDeviceConfig())
    {
        if (config->enable || this->find(config->idVendor, config->idProduct))
        {
            continue;
        }

        QSharedPointer<DeviceRule> devRule = QSharedPointer<DeviceRule>(new DeviceRule());

        devRule->idVendor = config->idVendor;
        devRule->idProduct = config->idProduct;
        devRule->read = config->read;
        devRule->write = config->write;
        devRule->execute = config->execute;
        devRule->interfaceType = config->interfaceType;

        m_rules.append(devRule);
    }

    this->updateToFile();
}

QSharedPointer<DeviceRule>
UdevRuleManager::find(const QString idVendor,
                      const QString idProduct)
{
    for (auto rule : m_rules)
    {
        if (rule->idVendor == idVendor &&
            rule->idProduct == idProduct)
        {
            return rule;
        }
    }

    return nullptr;
}

void UdevRuleManager::updateToFile()
{
    QStringList rules;

    for (auto rule : m_rules)
    {
        auto udevRule = this->rulleObj2Str(rule);
        if (udevRule.isNull())
        {
            continue;
        }

        rules.append(udevRule);
    }

    this->saveToFile(rules);
}

void UdevRuleManager::saveToFile(const QStringList &lines)
{
    QFile::remove(KSC_DEVICE_UDEV_RULES_FILE);

    RETURN_IF_TRUE(lines.isEmpty())

    QFile file(KSC_DEVICE_UDEV_RULES_FILE);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        KLOG_WARNING() << "Cannot open file " << KSC_DEVICE_UDEV_RULES_FILE;
        return;
    }

    QTextStream out(&file);
    auto context = lines.join("\n");
    out << context << "\n";

    file.close();
}

QString UdevRuleManager::getUdevModeValue(QSharedPointer<DeviceRule> rule)
{
    auto permission = rule->read * PER_BIN_VALUE_READ +
                      rule->write * PER_BIN_VALUE_WRITE +
                      rule->execute * PER_BIN_VALUE_EXECUTE;

    // 所有用户的读，写，执行权限一样
    return QString::asprintf("0%d%d%d", permission, permission, permission);
}

QString UdevRuleManager::rulleObj2Str(QSharedPointer<DeviceRule> rule)
{
    if (rule->interfaceType == INTERFACE_TYPE_USB)
    {
        return QString("ACTION!=\"remove\", SUBSYSTEMS==\"usb\", \
ATTRS{idVendor}==\"%1\", ATTRS{idProduct}==\"%2\", \
MODE=\"%3\", RUN=\"/bin/sh -c 'echo 0 >/sys/$devpath/authorized'\"")
            .arg(
                rule->idVendor,
                rule->idProduct,
                this->getUdevModeValue(rule));
    }

    return QString();
}

}  // namespace KS
