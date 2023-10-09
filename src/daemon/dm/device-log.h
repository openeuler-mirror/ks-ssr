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

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QSettings>

namespace KS
{
namespace DM
{
struct DeviceRecord
{
public:
    DeviceRecord() = default;
    QString name;
    int type;
    qint64 time;
    int state;
};

class DeviceLog : public QObject
{
    Q_OBJECT

public:
    DeviceLog(QObject *parent = nullptr);
    ~DeviceLog();

public:
    void addDeviceRecord(const DeviceRecord &record);
    QJsonObject toJsonObject(QSharedPointer<DeviceRecord> record);
    QList<QSharedPointer<DeviceRecord>> getDeviceRecords();

private:
    QSharedPointer<DeviceRecord> getDeviceRecord(const QString &group);
    void removeLastDeviceLog();

private:
    QSettings *m_settings;
};
}  // namespace DM
}  // namespace KS