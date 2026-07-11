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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#ifndef UDEV_DEVICE_ENUMERATOR_H
#define UDEV_DEVICE_ENUMERATOR_H

#include <QObject>

template <typename T>
class QList;
class udev;
class udev_enumerate;

namespace KS
{
namespace DM
{
class UdevDevice;

class UdevDeviceEnumerator : public QObject
{
    Q_OBJECT
public:
    UdevDeviceEnumerator();
    virtual ~UdevDeviceEnumerator();
    QList<UdevDevice*> getDevices() const;

private:
    udev_enumerate* m_deviceEnum;
    QList<UdevDevice*> m_devices;
};
}  // namespace DM
}  // namespace KS

#endif