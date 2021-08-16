/**
 * @file          /kiran-ssr-manager/lib/core/ssr-plugin-loader.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "lib/base/base.h"
#include "plugin-i.h"

namespace Kiran
{
class SSRPluginLoader
{
public:
    // 加载插件，获取插件句柄
    virtual bool load() = 0;
    // 激活插件，调用插件的激活函数，进行插件初始化
    virtual bool activate() = 0;
    // 取消激活，调用插件的取消激活函数，释放所有对象
    virtual bool deactivate() = 0;
    // 获取插件接口
    virtual std::shared_ptr<SSRPluginInterface> get_interface() = 0;
};

class SSRPluginCPPLoader : public SSRPluginLoader
{
public:
    SSRPluginCPPLoader(const std::string &so_path);
    virtual ~SSRPluginCPPLoader(){};

    virtual bool load() override;
    virtual bool activate() override;
    virtual bool deactivate() override;
    virtual std::shared_ptr<SSRPluginInterface> get_interface() override { return this->interface_; };

private:
    bool load_module();

private:
    // so文件路径
    std::string so_path_;

    // 是否已经激活
    bool is_activate_;

    std::shared_ptr<Glib::Module> module_;
    std::shared_ptr<SSRPluginInterface> interface_;
};

// TODO：暂不支持
class SSRPluginPythonLoader : public SSRPluginLoader
{
public:
    virtual bool load() override { return false; };
    virtual bool activate() override { return false; };
    virtual bool deactivate() override { return false; };
    virtual std::shared_ptr<SSRPluginInterface> get_interface() override { return nullptr; };
};

}  // namespace Kiran