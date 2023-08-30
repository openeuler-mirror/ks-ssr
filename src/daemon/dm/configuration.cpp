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
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#include "src/daemon/dm/configuration.h"
#include <qt5-log-i.h>
#include <QFile>
#include <QMutex>
#include <QProcess>
#include <QTextStream>
#include <QThread>
#include "config.h"
#include "src/daemon/common/systemd-proxy.h"
#include "src/daemon/dm/device-manager.h"
#include "ssr-i.h"
#include "ssr-marcos.h"

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
// 临时文件路径
#define TMP_PATH "/tmp/ks-ssr"
// 临时的 grub 配置文件， 用于分发至真正使用的 grub 配置文件。
#define TMP_GRUB_CFG_FILE_PATH "/tmp/ks-ssr/grub.cfg"
// legacy 的 grub 配置路径
#define GRUB_LEGACY_FILE_PATH "/boot/grub2/grub.cfg"
// efi 模式下的 grub 配置路径
#define GRUB_EFI_FILE_PATH "/boot/efi/EFI/KylinSecOS/grub.cfg"

Configuration *Configuration::instance()
{
    static QScopedPointer<Configuration> pInst;
    if (Q_UNLIKELY(!pInst))
    {
        if (pInst.isNull())
        {
            pInst.reset(new Configuration());
        }
    }
    return pInst.data();
}

void Configuration::addSetting(const DeviceSetting &setting)
{
    m_deviceSettings->beginGroup(setting.uid);
    m_deviceSettings->setValue(DEVICE_SK_ID, setting.id);
    m_deviceSettings->setValue(DEVICE_SK_NAME, setting.name);
    m_deviceSettings->setValue(DEVICE_SK_ID_PRODUCT, setting.idProduct);
    m_deviceSettings->setValue(DEVICE_SK_ID_VENDOR, setting.idVendor);
    m_deviceSettings->setValue(DEVICE_SK_TYPE, setting.type);
    m_deviceSettings->setValue(DEVICE_SK_INTERFACE_TYPE, setting.interfaceType);
    m_deviceSettings->setValue(DEVICE_SK_READ, setting.read);
    m_deviceSettings->setValue(DEVICE_SK_WRITE, setting.write);
    m_deviceSettings->setValue(DEVICE_SK_EXECUTE, setting.execute);
    m_deviceSettings->setValue(DEVICE_SK_ENABLE, setting.enable);
    m_deviceSettings->endGroup();

    Q_EMIT this->deviceSettingChanged();
}

QSharedPointer<DeviceSetting> Configuration::getDeviceSetting(const QString &uid)
{
    RETURN_VAL_IF_FALSE(m_deviceSettings->childGroups().contains(uid), nullptr)

    auto setting = QSharedPointer<DeviceSetting>(new DeviceSetting());

    m_deviceSettings->beginGroup(uid);

    setting->uid = uid;
    setting->id = m_deviceSettings->value(DEVICE_SK_ID).toString();
    setting->name = m_deviceSettings->value(DEVICE_SK_NAME).toString();
    setting->idProduct = m_deviceSettings->value(DEVICE_SK_ID_PRODUCT).toString();
    setting->idVendor = m_deviceSettings->value(DEVICE_SK_ID_VENDOR).toString();
    setting->interfaceType = m_deviceSettings->value(DEVICE_SK_INTERFACE_TYPE).toInt();
    setting->type = m_deviceSettings->value(DEVICE_SK_TYPE).toInt();
    setting->enable = m_deviceSettings->value(DEVICE_SK_ENABLE).toBool();
    setting->read = m_deviceSettings->value(DEVICE_SK_READ).toBool();
    setting->write = m_deviceSettings->value(DEVICE_SK_WRITE).toBool();
    setting->execute = m_deviceSettings->value(DEVICE_SK_EXECUTE).toBool();

    m_deviceSettings->endGroup();

    return setting;
}

bool Configuration::isIFCEnable(int type)
{
    auto group = QString::asprintf("interface%d", type);
    bool ret = false;

    // 未配置情况下，接口为启用状态
    RETURN_VAL_IF_FALSE(m_interfaceSettings->childGroups().contains(group), true)

    // FIXME: 为了 HDMI 接口禁用所特例化的功能
    RETURN_VAL_IF_TRUE(INTERFACE_TYPE_HDMI == type, m_isEnableHDMI);

    m_interfaceSettings->beginGroup(group);
    ret = m_interfaceSettings->value(DI_SK_ENABLE).toBool();
    m_interfaceSettings->endGroup();

    return ret;
}

void Configuration::setIFCEnable(int type, bool enable)
{
    if (type == INTERFACE_TYPE_USB &&
        enable)
    {
        // 开启USB口时，一起开启键盘，鼠标
        this->setIFCEnable(INTERFACE_TYPE_USB_KBD, true);
        this->setIFCEnable(INTERFACE_TYPE_USB_MOUSE, true);
    }

    QString group = QString::asprintf("interface%d", type);

#ifdef _345_GC_
    // FIXME: 由于 HDMI 接口的禁用需要修改内核参数导致的特殊处理，下个版本将内核参数修改的操作改成开机和关机时自动运行
    if (type == INTERFACE_TYPE_HDMI)
    {
        m_isEnableHDMI = enable;
        this->syncInterfaceToGrubFile();
        return;
    }
#endif

    m_interfaceSettings->beginGroup(group);
    m_interfaceSettings->setValue(DI_SK_TYPE, type);
    m_interfaceSettings->setValue(DI_SK_ENABLE, enable);
    m_interfaceSettings->endGroup();

    switch (type)
    {
#ifdef _345_GC_
    case INTERFACE_TYPE_HDMI:
        this->syncInterfaceToGrubFile();
        break;
#endif
    case INTERFACE_TYPE_BLUETOOTH:
        this->syncToBluetoothService();
        break;
    case INTERFACE_TYPE_NET:
        this->syncToNMService();
        break;
    }
}

Configuration::Configuration(QObject *parent) : QObject(parent),
                                                m_deviceSettings(nullptr),
                                                m_interfaceSettings(nullptr),
                                                m_grubUpdateThread(nullptr),
                                                m_waitingUpdateGrub(false)
{
    this->init();
}

Configuration::~Configuration()
{
}

void Configuration::init()
{
    m_deviceSettings = new QSettings(SSR_DEVICE_CONFIG_FILE, QSettings::NativeFormat, this);
    m_interfaceSettings = new QSettings(SSR_DI_CONFIG_FILE, QSettings::NativeFormat, this);
    // FIXME: 为了 HDMI 接口的特殊化处理
    auto group = QString("interface%1").arg(INTERFACE_TYPE_HDMI);
    auto isContainGroup = m_interfaceSettings->childGroups().contains(group);
    m_interfaceSettings->beginGroup(group);
    if (isContainGroup)
    {
        m_isEnableHDMI = m_interfaceSettings->value(DI_SK_ENABLE).toBool();
    }
    else
    {
        m_isEnableHDMI = true;
    }
    // 当配置文件中 HDMI 配置，优先使用配置文件的配置，没有配置时默认值为 true
    m_interfaceSettings->endGroup();

    // this->syncInterfaceFile();
}

void Configuration::syncInterfaceFile()
{
#ifdef _345_GC_
    this->syncInterfaceToGrubFile();
#endif
    this->syncToBluetoothService();
    this->syncToNMService();
}
#ifdef _345_GC_
void Configuration::syncInterfaceToGrubFile()
{
    QString grubValue;
    QString grubOption;

    // 生成grub选项
    auto hdmiNames = getHDMINames();
    // FIXME: 为了 HDMI 接口的特殊化处理
    // auto enabled = isIFCEnable(InterfaceType::INTERFACE_TYPE_HDMI);
    if (!m_isEnableHDMI && hdmiNames.size() != 0)
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
    QFile grubFile(SSR_DI_GRUB_FILE);
    if (!grubFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KLOG_WARNING() << "Open file " << SSR_DI_GRUB_FILE << " failed.";
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
    this->saveToFile(lines, SSR_DI_GRUB_FILE);

    m_waitingUpdateGrub = true;
    this->checkWaitingUpdateGrubs();
}
#endif

void Configuration::updateGrubsInThread()
{
    this->updateGrub(TMP_GRUB_CFG_FILE_PATH);
    QFileInfo legacyCfgFilePath(GRUB_LEGACY_FILE_PATH);
    QFileInfo efiCfgFilePath(GRUB_EFI_FILE_PATH);
    if (legacyCfgFilePath.isFile())
    {
        QFile::remove(GRUB_LEGACY_FILE_PATH);
        QFile::copy(TMP_GRUB_CFG_FILE_PATH, GRUB_LEGACY_FILE_PATH);
    }
    if (efiCfgFilePath.isFile())
    {
        QFile::remove(GRUB_EFI_FILE_PATH);
        QFile::copy(TMP_GRUB_CFG_FILE_PATH, GRUB_EFI_FILE_PATH);
    }
}

void Configuration::checkWaitingUpdateGrubs()
{
    RETURN_IF_TRUE(m_grubUpdateThread);

    if (m_waitingUpdateGrub)
    {
        m_waitingUpdateGrub = false;
        m_grubUpdateThread = QThread::create(std::bind(&Configuration::updateGrubsInThread, this));
        connect(m_grubUpdateThread, &QThread::finished, std::bind(&Configuration::finishGrubsUpdate, this));
        m_grubUpdateThread->start();
    }
}

void Configuration::finishGrubsUpdate()
{
    this->m_grubUpdateThread->deleteLater();
    this->m_grubUpdateThread = nullptr;
    this->checkWaitingUpdateGrubs();

    // FIXME: 为了 HDMI 接口的特殊化处理
    // 当更新内核参数配置命令更新完毕后再更新配置文件的标志位
    m_interfaceSettings->beginGroup(QString("interface%1").arg(INTERFACE_TYPE_HDMI));
    m_interfaceSettings->setValue(DI_SK_TYPE, INTERFACE_TYPE_HDMI);
    m_interfaceSettings->setValue(DI_SK_ENABLE, m_isEnableHDMI);
    m_interfaceSettings->endGroup();
}

QStringList Configuration::getHDMINames()
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

void Configuration::syncToBluetoothService()
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

void Configuration::syncToNMService()
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

void Configuration::saveToFile(const QStringList &lines, const QString &filepath)
{
    QFile::remove(filepath);

    RETURN_IF_TRUE(lines.isEmpty())

    QFile file(filepath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        KLOG_WARNING() << "Cannot open file " << filepath;
        return;
    }

    QTextStream out(&file);
    auto context = lines.join("\n");
    out << context << "\n";

    file.close();
}

void Configuration::updateGrub(const QString &filePath)
{
    QDir tmpPath(TMP_PATH);
    // FIXME: 下个版本将更新 grub 配置的命令替换为 grubby ，此命令不需要检查文件是否存在。
    if (!tmpPath.exists())
    {
        tmpPath.mkdir(TMP_PATH);
    }
    auto arguments = QStringList{QString("-o"), filePath};
    auto command = QString("%1 %2").arg(GRUB_MKCONFIG_PROGRAM).arg(arguments.join(' '));

    KLOG_DEBUG() << "Start run command " << command;
    auto exitcode = QProcess::execute(GRUB_MKCONFIG_PROGRAM, arguments);
    if (exitcode != 0)
    {
        KLOG_WARNING() << "Failed to execute command " << command << ", exitcode is " << exitcode;
    }
}

DeviceSettingList Configuration::getDeviceSettings()
{
    DeviceSettingList settings;
    auto groups = m_deviceSettings->childGroups();

    Q_FOREACH (auto group, groups)
    {
        QSharedPointer<DeviceSetting> setting = this->getDeviceSetting(group);

        if (setting)
        {
            settings.append(setting);
        }
    }

    return settings;
}

}  // namespace KS
