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
#include "sc-i.h"
#include "sc-marcos.h"

namespace KS
{

#define DEVICE_SETTINGS_KYE_ID "id"
#define DEVICE_SETTINGS_KYE_NAME "name"
#define DEVICE_SETTINGS_KYE_ID_PRODUCT "idProduct"
#define DEVICE_SETTINGS_KYE_ID_VENDOR "idVendor"
#define DEVICE_SETTINGS_KYE_TYPE "type"
#define DEVICE_SETTINGS_KYE_INTERFACE_TYPE "interfaceType"
#define DEVICE_SETTINGS_KYE_READ "read"
#define DEVICE_SETTINGS_KYE_WRITE "write"
#define DEVICE_SETTINGS_KYE_EXECUTE "execute"
#define DEVICE_SETTINGS_KYE_ENABLE "enable"

#define PERMISSION_BIN_VALUE_READ 4     // 2的2次方
#define PERMISSION_BIN_VALUE_WRITE 2    // 2的1次方
#define PERMISSION_BIN_VALUE_EXECUTE 1  // 2的0次方

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
    m_settings = new QSettings(SC_DEVICE_RULE_FILE, QSettings::NativeFormat, this);

    this->updateUdevFile();
}

void DeviceRule::addRule(const Rule &rule)
{
    m_settings->beginGroup(rule.uid);
    m_settings->setValue(DEVICE_SETTINGS_KYE_ID, rule.id);
    m_settings->setValue(DEVICE_SETTINGS_KYE_NAME, rule.name);
    m_settings->setValue(DEVICE_SETTINGS_KYE_ID_PRODUCT, rule.idProduct);
    m_settings->setValue(DEVICE_SETTINGS_KYE_ID_VENDOR, rule.idVendor);
    m_settings->setValue(DEVICE_SETTINGS_KYE_TYPE, rule.type);
    m_settings->setValue(DEVICE_SETTINGS_KYE_INTERFACE_TYPE, rule.interfaceType);
    m_settings->setValue(DEVICE_SETTINGS_KYE_READ, rule.read);
    m_settings->setValue(DEVICE_SETTINGS_KYE_WRITE, rule.write);
    m_settings->setValue(DEVICE_SETTINGS_KYE_EXECUTE, rule.execute);
    m_settings->setValue(DEVICE_SETTINGS_KYE_ENABLE, rule.enable);
    m_settings->endGroup();

    this->updateUdevFile();
}

QStringList DeviceRule::toUdevRules()
{
    auto groups = m_settings->childGroups();
    QStringList rules;

    Q_FOREACH (auto group, groups)
    {
        auto udevRule = this->rule2UdevRule(this->getRule(group));

        if(!udevRule.isNull())
        {
            rules.append(udevRule);
        }
    }

    return rules;
}

QString DeviceRule::rule2UdevRule(QSharedPointer<Rule> rule)
{
    if (rule->interfaceType == INTERFACE_TYPE_USB)
    {
        return QString::asprintf("ACTION==\"*\", SUBSYSTEMS==\"usb\", \
ATTRS{idVendor}==\"%s\", ATTRS{idProduct}==\"%s\", \
MODE=\"%s\", RUN=\"/bin/sh -c 'echo %d >/sys/$devpath/authorized'\"",
                                 rule->idVendor.toStdString().c_str(),
                                 rule->idVendor.toStdString().c_str(),
                                 this->getUdevModeValue(rule).toStdString().c_str(),
                                 rule->enable ? 1 : 0);
    }

    return QString();
}

QString DeviceRule::getUdevModeValue(QSharedPointer<Rule> rule)
{
    auto permission = rule->read * PERMISSION_BIN_VALUE_READ +
                      rule->write * PERMISSION_BIN_VALUE_WRITE +
                      rule->execute * PERMISSION_BIN_VALUE_EXECUTE;

    // 所有用户的读，写，执行权限一样
    return QString::asprintf("0%d%d%d", permission, permission, permission);
}

void DeviceRule::updateUdevFile()
{
    // 触发系统事件，让systemd-udevd服务重新加载规则文件
    QFile::remove(SC_DEVICE_UDEV_RULES_FILE);

    auto rules = this->toUdevRules();

    RETURN_IF_TRUE(rules.isEmpty())

    QFile file(SC_DEVICE_UDEV_RULES_FILE);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        KLOG_ERROR() << "Can not open file: ." << SC_DEVICE_UDEV_RULES_FILE;
        return;
    }

    QTextStream out(&file);

    auto context = rules.join("\n");

    out << context << "\n";

    file.close();
}

QSharedPointer<Rule> DeviceRule::getRule(const QString &uid)
{
    RETURN_VAL_IF_FALSE(this->groupExisted(uid), nullptr)

    auto rule = QSharedPointer<Rule>(new Rule());

    m_settings->beginGroup(uid);

    rule->uid = uid;
    rule->id = m_settings->value(DEVICE_SETTINGS_KYE_ID).toString();
    rule->name = m_settings->value(DEVICE_SETTINGS_KYE_NAME).toString();
    rule->idProduct = m_settings->value(DEVICE_SETTINGS_KYE_ID_PRODUCT).toString();
    rule->idVendor = m_settings->value(DEVICE_SETTINGS_KYE_ID_VENDOR).toString();
    rule->interfaceType = m_settings->value(DEVICE_SETTINGS_KYE_INTERFACE_TYPE).toInt();
    rule->type = m_settings->value(DEVICE_SETTINGS_KYE_TYPE).toInt();
    rule->enable = m_settings->value(DEVICE_SETTINGS_KYE_ENABLE).toBool();
    rule->read = m_settings->value(DEVICE_SETTINGS_KYE_READ).toBool();
    rule->write = m_settings->value(DEVICE_SETTINGS_KYE_WRITE).toBool();
    rule->execute = m_settings->value(DEVICE_SETTINGS_KYE_EXECUTE).toBool();

    m_settings->endGroup();

    return rule;
}

bool DeviceRule::groupExisted(const QString group)
{
    auto childGroups = m_settings->childGroups();

    Q_FOREACH (auto childGroup, childGroups)
    {
        if (childGroup == group)
        {
            return true;
        }
    }

    return false;
}

}  // namespace KS