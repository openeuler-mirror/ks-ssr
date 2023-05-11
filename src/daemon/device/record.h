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

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QSettings>

namespace KS
{
struct DeviceConnectRecord
{
public:
    DeviceConnectRecord() = default;
    QString name;
    int type;
    qint64 time;
    int state;
};

class Record : public QObject
{
    Q_OBJECT

public:
    Record(QObject *parent = nullptr);
    ~Record();

public:
    void addDeviceConnectRecord(const DeviceConnectRecord &record);
    QJsonObject toJsonObject(QSharedPointer<DeviceConnectRecord> record);
    QList<QSharedPointer<DeviceConnectRecord>> getDeviceConnectRecords();

private:
    QSharedPointer<DeviceConnectRecord> getDeviceConnectRecord(const QString &group);
    void removeLastRecord();

private:
    QSettings *m_settings;
};
}  // namespace KS