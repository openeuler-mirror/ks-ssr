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

#ifndef UDEV_DEVICE_DEVICE_H
#define UDEV_DEVICE_DEVICE_H

#include <QObject>

class udev_device;
namespace KS
{
namespace DM
{
class UdevDevice : public QObject
{
    Q_OBJECT
public:
    UdevDevice(udev_device* device);
    UdevDevice(const UdevDevice& device);
    UdevDevice(const QString& syspath);
    UdevDevice operator=(const UdevDevice& device);
    virtual ~UdevDevice();

public:
    QString getSyspath() const;
    QString getSubsystem() const;
    QString getDevtype() const;
    // udev 中没有 get_dev_name 接口， 并且此接口没有被使用，暂且忽略
    QString getDevname() const;
    QString getSysname() const;
    QString getSysattrValue(const QString& attr) const;
    QString getDevNode() const;
    void trigger();

private:
    udev_device* m_device;
};
}  // namespace DM
}  // namespace KS

#endif