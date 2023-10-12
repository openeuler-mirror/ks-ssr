/**
 * Copyright (c) 2022 ~ 2023 KylinSec Co., Ltd. 
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QObject>
#include <QSharedPointer>

namespace KS
{
class SystemdProxy : public QObject
{
    Q_OBJECT

public:
    SystemdProxy();
    virtual ~SystemdProxy(){};

    static QSharedPointer<SystemdProxy> getDefault();

    // 停止并禁用服务
    void stopAndDisableUnit(const QString &name);
    // 禁用服务
    void disableUnit(const QString &name);
    // 停止服务
    void stopUnit(const QString &name);

    // 开始并自启动服务
    void startAndEnableUnit(const QString &name);
    // 自启动服务
    void enableUnit(const QString &name);
    // 开始服务
    void startUnit(const QString &name);

private:
    void reload();
    // 调用函数不等待返回值
    void callNoReplay(const QString &method, const QList<QVariant> &arguments);

private:
    static QSharedPointer<SystemdProxy> m_instance;
};

}  // namespace KS
