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

#include <QObject>

typedef struct sd_device sd_device;

namespace KS
{
class SDDevice : public QObject
{
    Q_OBJECT

public:
    SDDevice(sd_device *device, QObject *parent = nullptr);
    SDDevice(const QString &syspath, QObject *parent = nullptr);
    virtual ~SDDevice();

public:
    QString getSyspath() const;
    QString getSubsystem() const;
    QString getDevtype() const;
    QString getDevname() const;
    QString getSysname() const;
    QString getSysattrValue(const QString &attr) const;
    void trigger();

private:
    sd_device *m_device;
};

}  // namespace KS