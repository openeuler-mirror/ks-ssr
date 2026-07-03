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
#include <QFileInfo>
#include <QMutex>
#include <QProcess>
#include <QTextStream>
#include <QThread>
#include "config.h"
#include "ksc-i.h"
#include "ksc-marcos.h"
#include "src/daemon/common/systemd-proxy.h"
#include "src/daemon/device/device-manager.h"

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

#define GRUB_MKCONFIG_PROGRAM "/usr/sbin/grub2-mkconfig"
// legacy模式下的grub配置路径
#define GRUB_LEGACY_FILE_PATH "/etc/grub2.cfg"
// efi模式下的grub配置路径
#define GRUB_EFI_FILE_PATH "/etc/grub2-efi.cfg"

DeviceRuleManager *DeviceRuleManager::instance()
{
    // TODO: 一个单线程为啥要整这么复杂
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

void DeviceRuleManager::addRule(const DeviceRule &rule)
{
    m_deviceSettings->beginGroup(rule.uid);
    m_deviceSettings->setValue(DEVICE_SK_ID, rule.id);
    m_deviceSettings->setValue(DEVICE_SK_NAME, rule.name);
    m_deviceSettings->setValue(DEVICE_SK_ID_PRODUCT, rule.idProduct);
    m_deviceSettings->setValue(DEVICE_SK_ID_VENDOR, rule.idVendor);
    m_deviceSettings->setValue(DEVICE_SK_TYPE, rule.type);
    m_deviceSettings->setValue(DEVICE_SK_INTERFACE_TYPE, rule.interfaceType);
    m_deviceSettings->setValue(DEVICE_SK_READ, rule.read);
    m_deviceSettings->setValue(DEVICE_SK_WRITE, rule.write);
    m_deviceSettings->setValue(DEVICE_SK_EXECUTE, rule.execute);
    m_deviceSettings->setValue(DEVICE_SK_ENABLE, rule.enable);
    m_deviceSettings->endGroup();

    this->syncDeviceFile();
}

QSharedPointer<DeviceRule> DeviceRuleManager::getRule(const QString &uid)
{
    RETURN_VAL_IF_FALSE(m_deviceSettings->childGroups().contains(uid), nullptr)

    auto rule = QSharedPointer<DeviceRule>(new DeviceRule());

    m_deviceSettings->beginGroup(uid);

    rule->uid = uid;
    rule->id = m_deviceSettings->value(DEVICE_SK_ID).toString();
    rule->name = m_deviceSettings->value(DEVICE_SK_NAME).toString();
    rule->idProduct = m_deviceSettings->value(DEVICE_SK_ID_PRODUCT).toString();
    rule->idVendor = m_deviceSettings->value(DEVICE_SK_ID_VENDOR).toString();
    rule->interfaceType = m_deviceSettings->value(DEVICE_SK_INTERFACE_TYPE).toInt();
    rule->type = m_deviceSettings->value(DEVICE_SK_TYPE).toInt();
    rule->enable = m_deviceSettings->value(DEVICE_SK_ENABLE).toBool();
    rule->read = m_deviceSettings->value(DEVICE_SK_READ).toBool();
    rule->write = m_deviceSettings->value(DEVICE_SK_WRITE).toBool();
    rule->execute = m_deviceSettings->value(DEVICE_SK_EXECUTE).toBool();

    m_deviceSettings->endGroup();

    return rule;
}

bool DeviceRuleManager::isIFCEnable(int type)
{
    auto group = QString::asprintf("interface%d", type);
    bool ret = false;

    // 未配置情况下，接口为启用状态
    RETURN_VAL_IF_FALSE(m_interfaceSettings->childGroups().contains(group), true)

    m_interfaceSettings->beginGroup(group);
    ret = m_interfaceSettings->value(DI_SK_ENABLE).toBool();
    m_interfaceSettings->endGroup();

    return ret;
}

void DeviceRuleManager::setIFCEnable(int type, bool enable)
{
    // RETURN_VAL_IF_FALSE(this->verifyInterface(type), false)

    QString group = QString::asprintf("interface%d", type);

    m_interfaceSettings->beginGroup(group);
    m_interfaceSettings->setValue(DI_SK_TYPE, type);
    m_interfaceSettings->setValue(DI_SK_ENABLE, enable);
    m_interfaceSettings->endGroup();

    this->syncInterfaceFile();
}

DeviceRuleManager::DeviceRuleManager(QObject *parent) : QObject(parent),
                                                        m_deviceSettings(nullptr),
                                                        m_interfaceSettings(nullptr),
                                                        m_grubUpdateThread(nullptr),
                                                        m_waitingUpdateGrub(false)
{
    this->init();
}

void DeviceRuleManager::init()
{
    m_deviceSettings = new QSettings(KSC_DEVICE_RULE_FILE, QSettings::NativeFormat, this);
    m_interfaceSettings = new QSettings(KSC_DI_RULE_FILE, QSettings::NativeFormat, this);

    this->syncDeviceFile();
    this->syncInterfaceFile();
}

void DeviceRuleManager::syncDeviceFile()
{
    this->syncToDeviceUdevFile();
}

void DeviceRuleManager::syncToDeviceUdevFile()
{
    QStringList rules;
    auto groups = m_deviceSettings->childGroups();

    Q_FOREACH (auto group, groups)
    {
        auto udevRule = this->ruleObj2Str(this->getRule(group));

        if (!udevRule.isNull())
        {
            rules.append(udevRule);
        }
    }
    this->saveToFile(rules, KSC_DEVICE_UDEV_RULES_FILE);
}

QString DeviceRuleManager::ruleObj2Str(QSharedPointer<DeviceRule> rule)
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

void DeviceRuleManager::syncInterfaceFile()
{
    this->syncToInterfaceUdevFile();
    this->syncInterfaceToGrubFile();
    this->syncToBluetoothService();
}

void DeviceRuleManager::syncToInterfaceUdevFile()
{
    QStringList rules;
    int type;

    // TODO: 需要修改代码，不是每个类型都有UDEV规则
    for (type = INTERFACE_TYPE_USB; type < INTERFACE_TYPE_LAST; type++)
    {
        auto udevRule = this->getInterfaceUdevRule(type);

        if (!udevRule.isNull())
        {
            rules.append(udevRule);
        }
    }

    this->saveToFile(rules, KSC_DI_UDEV_RULE_FILE);
}

QString DeviceRuleManager::getInterfaceUdevRule(int type)
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

void DeviceRuleManager::syncInterfaceToGrubFile()
{
    QString grubValue;
    QString grubOption;

    // 生成grub选项
    auto hdmiNames = getHDMINames();
    auto enabled = isIFCEnable(InterfaceType::INTERFACE_TYPE_HDMI);
    if (!enabled && hdmiNames.size() != 0)
    {
        for (const auto &hdmiName : hdmiNames)
        {
            if (!grubValue.isEmpty())
            {
                grubValue.append(' ');
            }
            grubValue.append(QString("video=%1:d").arg(hdmiName));
        }
    }
    grubOption = QString("GRUB_CMDLINE_LINUX_DEFAULT=\"%1\"").arg(grubValue);

    // 读取grub配置并替换掉对应的grub选项
    QFile grubFile(KSC_DI_GRUB_FILE);
    if (!grubFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KLOG_WARNING() << "Open file " << KSC_DI_GRUB_FILE << " failed.";
        return;
    }

    QStringList lines;
    QTextStream grubIn(&grubFile);
    while (!grubIn.atEnd())
    {
        QString line = grubIn.readLine();
        if (!line.contains("GRUB_CMDLINE_LINUX_DEFAULT"))
        {
            lines.append(line);
        }
    }
    lines.append(grubOption);
    grubFile.close();
    this->saveToFile(lines, KSC_DI_GRUB_FILE);

    m_waitingUpdateGrub = true;
    this->checkWaitingUpdateGrubs();
}

void DeviceRuleManager::updateGrubsInThread()
{
    this->updateGrub(GRUB_LEGACY_FILE_PATH);
    this->updateGrub(GRUB_EFI_FILE_PATH);
}

void DeviceRuleManager::checkWaitingUpdateGrubs()
{
    RETURN_IF_TRUE(m_grubUpdateThread);

    if (m_waitingUpdateGrub)
    {
        m_waitingUpdateGrub = false;
        m_grubUpdateThread = QThread::create(std::bind(&DeviceRuleManager::updateGrubsInThread, this));
        connect(m_grubUpdateThread, &QThread::finished, std::bind(&DeviceRuleManager::finishGrubsUpdate, this));
        m_grubUpdateThread->start();
    }
}

void DeviceRuleManager::finishGrubsUpdate()
{
    this->m_grubUpdateThread->deleteLater();
    this->m_grubUpdateThread = nullptr;
    this->checkWaitingUpdateGrubs();
}

QStringList DeviceRuleManager::getHDMINames()
{
    QStringList hdmiNames;
    auto hdmiDevices = DeviceManager::instance()->getDevicesByInterface(InterfaceType::INTERFACE_TYPE_HDMI);
    for (auto hdmiDevice : hdmiDevices)
    {
        auto syspath = hdmiDevice->getSyspath();
        auto syspathBaseName = QFileInfo(syspath).baseName();
        auto hdmiPos = syspathBaseName.indexOf("HDMI");
        if (hdmiPos >= 0)
        {
            hdmiNames.push_back(syspathBaseName.right(syspathBaseName.size() - hdmiPos));
        }
    }
    return hdmiNames;
}

void DeviceRuleManager::syncToBluetoothService()
{
    auto enabled = isIFCEnable(InterfaceType::INTERFACE_TYPE_BLUETOOTH);
    if (enabled)
    {
        SystemdProxy::getDefault()->startAndEnableUnit("bluetooth.service");
    }
    else
    {
        SystemdProxy::getDefault()->stopAndDisableUnit("bluetooth.service");
    }
}

void DeviceRuleManager::saveToFile(const QStringList &lines, const QString &filename)
{
    // 触发系统事件，让systemd-udevd服务重新加载规则文件
    QFile::remove(filename);

    RETURN_IF_TRUE(lines.isEmpty())

    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        KLOG_WARNING() << "Cannot open file " << filename;
        return;
    }

    QTextStream out(&file);
    auto context = lines.join("\n");
    out << context << "\n";

    file.close();
}

void DeviceRuleManager::updateGrub(const QString &filePath)
{
    auto arguments = QStringList{QString("-o"), filePath};
    auto command = QString("%1 %2").arg(GRUB_MKCONFIG_PROGRAM).arg(arguments.join(' '));

    KLOG_DEBUG() << "Start run command " << command;
    auto exitcode = QProcess::execute(GRUB_MKCONFIG_PROGRAM, arguments);
    if (exitcode != 0)
    {
        KLOG_WARNING() << "Failed to execute command " << command << ", exitcode is " << exitcode;
    }
}

}  // namespace KS
