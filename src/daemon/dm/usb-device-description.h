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

#pragma once

#include <QMap>
#include <QObject>

namespace KS
{
namespace DM
{
class USBDeviceDescription : public QObject
{
    Q_OBJECT

public:
    static USBDeviceDescription *instance();

    QString getManufacturerDesc(const QString idVendor) const;
    QString getProductDesc(const QString idVendor, const QString idProduct) const;

private:
    explicit USBDeviceDescription(QObject *parent = nullptr);

private:
    void init();
    void processDescriptionLine(const QString idLine);

private:
    QMap<QString, QString> m_descs;
    QString m_prevVendorID;
};
}  // namespace DM
}  // namespace KS
