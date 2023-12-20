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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#pragma once

#include <QLibrary>
#include <QSharedPointer>
#include <QString>
#include "include/br-plugin-i.h"
#include "lib/base/base.h"

namespace KS
{
namespace BRDaemon
{
class PluginLoader
{
public:
    // 加载插件，获取插件句柄
    virtual bool load() = 0;
    // 激活插件，调用插件的激活函数，进行插件初始化
    virtual bool activate() = 0;
    // 取消激活，调用插件的取消激活函数，释放所有对象
    virtual bool deactivate() = 0;
    // 获取插件接口
    virtual QSharedPointer<BRPluginInterface> getInterface() = 0;
};

class PluginCPPLoader : public PluginLoader
{
public:
    PluginCPPLoader(const QString &so_path);
    virtual ~PluginCPPLoader(){};

    virtual bool load() override;
    virtual bool activate() override;
    virtual bool deactivate() override;
    virtual QSharedPointer<BRPluginInterface> getInterface() override
    {
        return this->interface_;
    };

private:
    bool load_module();

private:
    // so文件路径
    QString so_path_;
    // 是否已经激活
    bool is_activate_;

    // QSharedPointer<Glib::Module> module_;
    QSharedPointer<QLibrary> module_;
    QSharedPointer<BRPluginInterface> interface_;
};

class PluginPythonLoader : public PluginLoader
{
public:
    PluginPythonLoader(const QString &package_name);
    virtual ~PluginPythonLoader(){};

public:
    virtual bool load() override;
    virtual bool activate() override;
    virtual bool deactivate() override;
    virtual QSharedPointer<BRPluginInterface> getInterface() override
    {
        return this->interface_;
    };

private:
    // 包名
    QString package_name_;
    // 是否已经激活
    bool is_activate_;
    QSharedPointer<BRPluginInterface> interface_;
};

}  // namespace BRDaemon
}  // namespace KS