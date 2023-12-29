/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVDescriptionED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#include "usb-device-description.h"
#include <qt5-log-i.h>
#include <QFile>
#include "config.h"
#include "ssr-marcos.h"

namespace KS
{
namespace DM
{
#define USB_ID_LENGTH 4

USBDeviceDescription *USBDeviceDescription::instance()
{
    static QScopedPointer<USBDeviceDescription> pInst;
    if (Q_UNLIKELY(!pInst))
    {
        if (pInst.isNull())
        {
            pInst.reset(new USBDeviceDescription());
        }
    }
    return pInst.data();
}

USBDeviceDescription::USBDeviceDescription(QObject *parent)
    : QObject(parent)
{
    m_prevVendorID = QString();
    this->init();
}

void USBDeviceDescription::init()
{
    QFile file(SSR_DEVICE_USB_DESC_FILE);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        KLOG_WARNING() << "Cannot open file " << SSR_DEVICE_USB_DESC_FILE;
        return;
    }

    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine();

        this->processDescriptionLine(line);
    }
}

void USBDeviceDescription::processDescriptionLine(const QString idLine)
{
    RETURN_IF_TRUE(idLine.contains("#") || idLine.isEmpty())

    auto strs = idLine.split("  ");

    RETURN_IF_TRUE(strs.length() != 2)

    auto id = strs.at(0).right(USB_ID_LENGTH);
    auto desc = strs.at(1);

    if (idLine.startsWith("\t"))
    {
        // 设备名称描述行
        m_descs.insert(QString("%1:%2").arg(m_prevVendorID, id), desc);
    }
    else
    {
        // 厂商名称描述行
        m_prevVendorID = id;
        m_descs.insert(id, desc);
    }
}

QString USBDeviceDescription::getManufacturerDesc(const QString idVendor) const
{
    return m_descs.value(idVendor);
}

QString USBDeviceDescription::getProductDesc(const QString idVendor, const QString idProduct) const
{
    return m_descs.value(QString("%1:%2").arg(idVendor, idProduct));
}
}  // namespace DM
}  // namespace KS