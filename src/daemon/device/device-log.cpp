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

#include "src/daemon/device/device-log.h"
#include <config.h>
#include "ssr-i.h"
#include "ssr-marcos.h"

namespace KS
{
#define DCR_KYE_NAME "name"
#define DCR_KYE_TYPE "type"
#define DCR_KYE_STATE "state"

#define MAX_CONNECT_RECORD_VALUE 1000

DeviceLog::DeviceLog(QObject *parent)
    : QObject(parent)
{
    m_settings = new QSettings(SSR_DEVICE_CONNECT_LOG_FILE, QSettings::NativeFormat, this);
}

DeviceLog::~DeviceLog()
{
}

void DeviceLog::addDeviceRecord(const DeviceRecord &record)
{
    removeLastDeviceLog();

    m_settings->beginGroup(QString::asprintf("%ld", record.time));
    m_settings->setValue(DCR_KYE_NAME, record.name);
    m_settings->setValue(DCR_KYE_TYPE, record.type);
    m_settings->setValue(DCR_KYE_STATE, record.state);
    m_settings->endGroup();
}

QList<QSharedPointer<DeviceRecord>> DeviceLog::getDeviceRecords()
{
    auto groups = m_settings->childGroups();
    QList<QSharedPointer<DeviceRecord>> records;

    Q_FOREACH (auto group, groups)
    {
        records.append(getDeviceRecord(group));
    }

    return records;
}

QSharedPointer<DeviceRecord> DeviceLog::getDeviceRecord(const QString &group)
{
    auto record = QSharedPointer<DeviceRecord>(new DeviceRecord());

    m_settings->beginGroup(group);

    record->time = group.toInt();

    record->name = m_settings->value(DCR_KYE_NAME).toString();
    record->type = m_settings->value(DCR_KYE_TYPE).toInt();
    record->state = m_settings->value(DCR_KYE_STATE).toInt();

    m_settings->endGroup();

    return record;
}

QJsonObject DeviceLog::toJsonObject(QSharedPointer<DeviceRecord> record)
{
    QJsonObject jsonObj{
        {SSR_DCR_JK_NAME, record->name},
        {SSR_DCR_JK_TYPE, record->type},
        {SSR_DCR_JK_TIME, record->time},
        {SSR_DCR_JK_STATE, record->state}};

    return jsonObj;
}

void DeviceLog::removeLastDeviceLog()
{
    auto groups = m_settings->childGroups();

    RETURN_IF_FALSE(groups.length() > MAX_CONNECT_RECORD_VALUE)

    auto group = groups.first();

    m_settings->remove(group);
}

}  // namespace KS
