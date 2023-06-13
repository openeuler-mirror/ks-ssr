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

#include "src/daemon/device/device-configuration.h"
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

#define GRUB_MKCONFIG_PROGRAM "/usr/sbin/grub2-mkconfig"
#define NMCLI_PROGRAM "/usr/bin/nmcli"
// legacy模式下的grub配置路径
#define GRUB_LEGACY_FILE_PATH "/etc/grub2.cfg"
// efi模式下的grub配置路径
#define GRUB_EFI_FILE_PATH "/etc/grub2-efi.cfg"

DeviceConfiguration *DeviceConfiguration::instance()
{
    static QScopedPointer<DeviceConfiguration> pInst;
    if (Q_UNLIKELY(!pInst))
    {
        if (pInst.isNull())
        {
            pInst.reset(new DeviceConfiguration());
        }
    }
    return pInst.data();
}

void DeviceConfiguration::addConfig(const DeviceConfig &config)
{
    m_deviceSettings->beginGroup(config.uid);
    m_deviceSettings->setValue(DEVICE_SK_ID, config.id);
    m_deviceSettings->setValue(DEVICE_SK_NAME, config.name);
    m_deviceSettings->setValue(DEVICE_SK_ID_PRODUCT, config.idProduct);
    m_deviceSettings->setValue(DEVICE_SK_ID_VENDOR, config.idVendor);
    m_deviceSettings->setValue(DEVICE_SK_TYPE, config.type);
    m_deviceSettings->setValue(DEVICE_SK_INTERFACE_TYPE, config.interfaceType);
    m_deviceSettings->setValue(DEVICE_SK_READ, config.read);
    m_deviceSettings->setValue(DEVICE_SK_WRITE, config.write);
    m_deviceSettings->setValue(DEVICE_SK_EXECUTE, config.execute);
    m_deviceSettings->setValue(DEVICE_SK_ENABLE, config.enable);
    m_deviceSettings->endGroup();

    Q_EMIT this->deviceConfigChanged();
}

QSharedPointer<DeviceConfig> DeviceConfiguration::getDeviceConfig(const QString &uid)
{
    RETURN_VAL_IF_FALSE(m_deviceSettings->childGroups().contains(uid), nullptr)

    auto config = QSharedPointer<DeviceConfig>(new DeviceConfig());

    m_deviceSettings->beginGroup(uid);

    config->uid = uid;
    config->id = m_deviceSettings->value(DEVICE_SK_ID).toString();
    config->name = m_deviceSettings->value(DEVICE_SK_NAME).toString();
    config->idProduct = m_deviceSettings->value(DEVICE_SK_ID_PRODUCT).toString();
    config->idVendor = m_deviceSettings->value(DEVICE_SK_ID_VENDOR).toString();
    config->interfaceType = m_deviceSettings->value(DEVICE_SK_INTERFACE_TYPE).toInt();
    config->type = m_deviceSettings->value(DEVICE_SK_TYPE).toInt();
    config->enable = m_deviceSettings->value(DEVICE_SK_ENABLE).toBool();
    config->read = m_deviceSettings->value(DEVICE_SK_READ).toBool();
    config->write = m_deviceSettings->value(DEVICE_SK_WRITE).toBool();
    config->execute = m_deviceSettings->value(DEVICE_SK_EXECUTE).toBool();

    m_deviceSettings->endGroup();

    return config;
}

bool DeviceConfiguration::isIFCEnable(int type)
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

void DeviceConfiguration::setIFCEnable(int type, bool enable)
{
    QString group = QString::asprintf("interface%d", type);

    m_interfaceSettings->beginGroup(group);
    m_interfaceSettings->setValue(DI_SK_TYPE, type);
    m_interfaceSettings->setValue(DI_SK_ENABLE, enable);
    m_interfaceSettings->endGroup();

    this->syncInterfaceFile();
}

DeviceConfiguration::DeviceConfiguration(QObject *parent) : QObject(parent),
                                                            m_deviceSettings(nullptr),
                                                            m_interfaceSettings(nullptr),
                                                            m_grubUpdateThread(nullptr),
                                                            m_waitingUpdateGrub(false)
{
    this->init();
}

void DeviceConfiguration::init()
{
    m_deviceSettings = new QSettings(KSC_DEVICE_CONFIG_FILE, QSettings::NativeFormat, this);
    m_interfaceSettings = new QSettings(KSC_DI_CONFIG_FILE, QSettings::NativeFormat, this);

    this->syncInterfaceFile();
}

void DeviceConfiguration::syncInterfaceFile()
{
    this->syncInterfaceToGrubFile();
    this->syncToBluetoothService();
    this->syncToNMService();
}

void DeviceConfiguration::syncInterfaceToGrubFile()
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

void DeviceConfiguration::updateGrubsInThread()
{
    this->updateGrub(GRUB_LEGACY_FILE_PATH);
    this->updateGrub(GRUB_EFI_FILE_PATH);
}

void DeviceConfiguration::checkWaitingUpdateGrubs()
{
    RETURN_IF_TRUE(m_grubUpdateThread);

    if (m_waitingUpdateGrub)
    {
        m_waitingUpdateGrub = false;
        m_grubUpdateThread = QThread::create(std::bind(&DeviceConfiguration::updateGrubsInThread, this));
        connect(m_grubUpdateThread, &QThread::finished, std::bind(&DeviceConfiguration::finishGrubsUpdate, this));
        m_grubUpdateThread->start();
    }
}

void DeviceConfiguration::finishGrubsUpdate()
{
    this->m_grubUpdateThread->deleteLater();
    this->m_grubUpdateThread = nullptr;
    this->checkWaitingUpdateGrubs();
}

QStringList DeviceConfiguration::getHDMINames()
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

void DeviceConfiguration::syncToBluetoothService()
{
    auto enabled = this->isIFCEnable(InterfaceType::INTERFACE_TYPE_BLUETOOTH);
    if (enabled)
    {
        SystemdProxy::getDefault()->startAndEnableUnit("bluetooth.service");
    }
    else
    {
        SystemdProxy::getDefault()->stopAndDisableUnit("bluetooth.service");
    }
}

void DeviceConfiguration::syncToNMService()
{
    auto enabled = this->isIFCEnable(InterfaceType::INTERFACE_TYPE_NET);
    auto arguments = QStringList{QString("n"), (enabled ? "on" : "off")};
    auto command = QString("%1 %2").arg(NMCLI_PROGRAM).arg(arguments.join(' '));

    KLOG_DEBUG() << "Sync switch to networkmanager service.";

    auto exitcode = QProcess::execute(NMCLI_PROGRAM, arguments);
    if (exitcode != 0)
    {
        KLOG_WARNING() << "Failed to execute command " << command << ", exitcode is " << exitcode;
    }
}

void DeviceConfiguration::saveToFile(const QStringList &lines, const QString &filename)
{
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

void DeviceConfiguration::updateGrub(const QString &filePath)
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

DeviceConfigList DeviceConfiguration::getDeviceConfig()
{
    DeviceConfigList configs;

    QStringList rules;
    auto groups = m_deviceSettings->childGroups();

    Q_FOREACH (auto group, groups)
    {
        QSharedPointer<DeviceConfig> config = this->getDeviceConfig(group);

        if (config)
        {
            configs.append(config);
        }
    }

    return configs;
}

}  // namespace KS
