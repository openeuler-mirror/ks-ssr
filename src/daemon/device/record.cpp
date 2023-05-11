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

#include "src/daemon/device/record.h"
#include <config.h>
#include "sc-i.h"
#include "sc-marcos.h"

namespace KS
{
#define DEVICE_CONNECT_RECORD_KYE_NAME "name"
#define DEVICE_CONNECT_RECORD_KYE_TYPE "type"
#define DEVICE_CONNECT_RECORD_KYE_STATE "state"

#define MAX_CONNECT_RECORD_VALUE 1000

Record::Record(QObject *parent)
    : QObject(parent)
{
    m_settings = new QSettings(SC_DEVICE_CONNECT_LOG_FILE, QSettings::NativeFormat, this);
}

Record::~Record()
{
}

void Record::addDeviceConnectRecord(const DeviceConnectRecord &record)
{
    this->removeLastRecord();
    
    m_settings->beginGroup(QString::asprintf("%ld", record.time));
    m_settings->setValue(DEVICE_CONNECT_RECORD_KYE_NAME, record.name);
    m_settings->setValue(DEVICE_CONNECT_RECORD_KYE_TYPE, record.type);
    m_settings->setValue(DEVICE_CONNECT_RECORD_KYE_STATE, record.state);
    m_settings->endGroup();
}

QList<QSharedPointer<DeviceConnectRecord>> Record::getDeviceConnectRecords()
{
    auto groups = m_settings->childGroups();
    QList<QSharedPointer<DeviceConnectRecord>> records;

    Q_FOREACH (auto group, groups)
    {
        records.append(this->getDeviceConnectRecord(group));
    }

    return records;
}

QSharedPointer<DeviceConnectRecord> Record::getDeviceConnectRecord(const QString &group)
{
    auto record = QSharedPointer<DeviceConnectRecord>(new DeviceConnectRecord());

    m_settings->beginGroup(group);

    record->time = group.toInt();

    record->name = m_settings->value(DEVICE_CONNECT_RECORD_KYE_NAME).toString();
    record->type = m_settings->value(DEVICE_CONNECT_RECORD_KYE_TYPE).toInt();
    record->state = m_settings->value(DEVICE_CONNECT_RECORD_KYE_STATE).toInt();

    m_settings->endGroup();

    return record;
}

QJsonObject Record::toJsonObject(QSharedPointer<DeviceConnectRecord> record)
{
    QJsonObject jsonObj{
        {SC_DEVICE_CONNECT_RECORD_KEY_NAME, record->name},
        {SC_DEVICE_CONNECT_RECORD_KEY_TYPE, record->type},
        {SC_DEVICE_CONNECT_RECORD_KEY_TIME, record->time},
        {SC_DEVICE_CONNECT_RECORD_KEY_STATE, record->state}};

    return jsonObj;
}

void Record::removeLastRecord()
{
    auto groups = m_settings->childGroups();

    RETURN_IF_FALSE(groups.length() > MAX_CONNECT_RECORD_VALUE)

    auto group = groups.first();

    m_settings->remove(group);
}

}  // namespace KS
