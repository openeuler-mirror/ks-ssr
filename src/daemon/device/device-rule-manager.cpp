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

#include "src/daemon/device/device-rule-manager.h"
#include <qt5-log-i.h>
#include <QFile>
#include <QMutex>
#include <QTextStream>
#include "config.h"
#include "ksc-i.h"
#include "ksc-marcos.h"

namespace KS
{

#define DEVICE_SK_ID "id"
#define DEVICE_SK_NAME "name"
#define DEVICE_SK_ID_PRODUCT "idProduct"
#define DEVICE_SK_ID_VENDOR "idVendor"
#define DEVICE_SK_TYPE "type"
#define DEVICE_SK_INTERFACE_TYPE "interfaceType"
#define DEVICE_SK_READ "read"
#define DEVICE_SK_WRITE "write"
#define DEVICE_SK_EXECUTE "execute"
#define DEVICE_SK_ENABLE "enable"

#define DI_SK_TYPE "type"
#define DI_SK_ENABLE "enable"

#define PER_BIN_VALUE_READ 4     // 2的2次方
#define PER_BIN_VALUE_WRITE 2    // 2的1次方
#define PER_BIN_VALUE_EXECUTE 1  // 2的0次方

DeviceRuleManager *DeviceRuleManager::instance()
{
    static QMutex mutex;
    static QScopedPointer<DeviceRuleManager> pInst;
    if (Q_UNLIKELY(!pInst))
    {
        QMutexLocker locker(&mutex);
        if (pInst.isNull())
        {
            pInst.reset(new DeviceRuleManager());
        }
    }
    return pInst.data();
}

DeviceRuleManager::DeviceRuleManager(QObject *parent) : QObject(parent)
{
    this->init();
}

void DeviceRuleManager::init()
{
    m_settings = new QSettings(KSC_DEVICE_RULE_FILE, QSettings::NativeFormat, this);
    m_ifcSettings = new QSettings(KSC_DI_RULE_FILE, QSettings::NativeFormat, this);

    this->updateUdevFile();
    this->updateIFCUdevFile();
}

void DeviceRuleManager::addRule(const DeviceRule &rule)
{
    m_settings->beginGroup(rule.uid);
    m_settings->setValue(DEVICE_SK_ID, rule.id);
    m_settings->setValue(DEVICE_SK_NAME, rule.name);
    m_settings->setValue(DEVICE_SK_ID_PRODUCT, rule.idProduct);
    m_settings->setValue(DEVICE_SK_ID_VENDOR, rule.idVendor);
    m_settings->setValue(DEVICE_SK_TYPE, rule.type);
    m_settings->setValue(DEVICE_SK_INTERFACE_TYPE, rule.interfaceType);
    m_settings->setValue(DEVICE_SK_READ, rule.read);
    m_settings->setValue(DEVICE_SK_WRITE, rule.write);
    m_settings->setValue(DEVICE_SK_EXECUTE, rule.execute);
    m_settings->setValue(DEVICE_SK_ENABLE, rule.enable);
    m_settings->endGroup();

    this->updateUdevFile();
}

QStringList DeviceRuleManager::getUdevRules()
{
    auto groups = m_settings->childGroups();
    QStringList rules;

    Q_FOREACH (auto group, groups)
    {
        auto udevRule = this->rule2UdevRule(this->getRule(group));

        if (!udevRule.isNull())
        {
            rules.append(udevRule);
        }
    }

    return rules;
}

QString DeviceRuleManager::rule2UdevRule(QSharedPointer<DeviceRule> rule)
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

QString DeviceRuleManager::getUdevModeValue(QSharedPointer<DeviceRule> rule)
{
    auto permission = rule->read * PER_BIN_VALUE_READ +
                      rule->write * PER_BIN_VALUE_WRITE +
                      rule->execute * PER_BIN_VALUE_EXECUTE;

    // 所有用户的读，写，执行权限一样
    return QString::asprintf("0%d%d%d", permission, permission, permission);
}

void DeviceRuleManager::updateUdevFile()
{
    this->saveToFile(this->getUdevRules(),
                     KSC_DEVICE_UDEV_RULES_FILE);
}

void DeviceRuleManager::saveToFile(const QStringList &rules,
                                   const QString &filename)
{
    RETURN_IF_TRUE(rules.isEmpty())

    // 触发系统事件，让systemd-udevd服务重新加载规则文件
    QFile::remove(filename);

    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        KLOG_WARNING() << "Can not open file: ." << filename;
        return;
    }

    QTextStream out(&file);
    auto context = rules.join("\n");
    out << context << "\n";

    file.close();
}

QSharedPointer<DeviceRule> DeviceRuleManager::getRule(const QString &uid)
{
    RETURN_VAL_IF_FALSE(this->groupExisted(uid), nullptr)

    auto rule = QSharedPointer<DeviceRule>(new DeviceRule());

    m_settings->beginGroup(uid);

    rule->uid = uid;
    rule->id = m_settings->value(DEVICE_SK_ID).toString();
    rule->name = m_settings->value(DEVICE_SK_NAME).toString();
    rule->idProduct = m_settings->value(DEVICE_SK_ID_PRODUCT).toString();
    rule->idVendor = m_settings->value(DEVICE_SK_ID_VENDOR).toString();
    rule->interfaceType = m_settings->value(DEVICE_SK_INTERFACE_TYPE).toInt();
    rule->type = m_settings->value(DEVICE_SK_TYPE).toInt();
    rule->enable = m_settings->value(DEVICE_SK_ENABLE).toBool();
    rule->read = m_settings->value(DEVICE_SK_READ).toBool();
    rule->write = m_settings->value(DEVICE_SK_WRITE).toBool();
    rule->execute = m_settings->value(DEVICE_SK_EXECUTE).toBool();

    m_settings->endGroup();

    return rule;
}

bool DeviceRuleManager::groupExisted(const QString group)
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

bool DeviceRuleManager::isIFCEnable(int type)
{
    auto group = QString::asprintf("interface%d", type);
    bool ret = false;

    // 未配置情况下，接口为启用状态
    RETURN_VAL_IF_FALSE(this->groupExisted(group), true)

    m_ifcSettings->beginGroup(group);
    ret = m_ifcSettings->value(DI_SK_ENABLE).toBool();
    m_ifcSettings->endGroup();

    return ret;
}

bool DeviceRuleManager::setIFCEnable(int type,
                                     bool enable)
{
   // RETURN_VAL_IF_FALSE(this->verifyInterface(type), false)

    QString group = QString::asprintf("interface%d", type);

    m_ifcSettings->beginGroup(group);
    m_ifcSettings->setValue(DI_SK_TYPE, type);
    m_ifcSettings->setValue(DI_SK_ENABLE, enable);
    m_ifcSettings->endGroup();

    this->updateIFCUdevFile();

    return true;
}

void DeviceRuleManager::updateIFCUdevFile()
{
    this->saveToFile(this->getIFCUdevRules(),
                     KSC_DI_UDEV_RULE_FILE);
}

QStringList DeviceRuleManager::getIFCUdevRules()
{
    QStringList rules;
    int type;

    for (type = INTERFACE_TYPE_USB; type < INTERFACE_TYPE_UNKNOWN; type++)
    {
        auto udevRule = this->getIFCUdevRule(type);

        if (!udevRule.isNull())
        {
            rules.append(udevRule);
        }
    }

    return rules;
}

QString DeviceRuleManager::getIFCUdevRule(int type)
{
    auto enable = this->isIFCEnable(type);

    switch (type)
    {
    case INTERFACE_TYPE_USB:
        return QString::asprintf("ACTION==\"*\", SUBSYSTEMS==\"usb\", \
RUN=\"/bin/sh -c 'echo %d >/sys/$devpath/authorized'\"",
                                 enable ? 1 : 0);

    case INTERFACE_TYPE_BLUETOOTH:
        break;

    case INTERFACE_TYPE_NET:
        return QString::asprintf("ACTION==\"*\", SUBSYSTEM==\"net\", SUBSYSTEMS==\"pci\", \
RUN=\"/bin/sh -c 'nmcli n %s'\"",
                                 enable ? "on" : "off");
    case INTERFACE_TYPE_HDMI:
        break;

    default:
        KLOG_WARNING() << "Unkown device interface type " << type;
        break;
    }

    return QString();
}

}  // namespace KS
