/**
 * @file          /ks-ssr-manager/src/daemon/plugin-loader.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */
#pragma once

#include "lib/base/base.h"

namespace KS
{
namespace Daemon
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
    virtual std::shared_ptr<SSRPluginInterface> get_interface() = 0;
};

class PluginCPPLoader : public PluginLoader
{
public:
    PluginCPPLoader(const std::string &so_path);
    virtual ~PluginCPPLoader(){};

    virtual bool load();
    virtual bool activate();
    virtual bool deactivate();
    virtual std::shared_ptr<SSRPluginInterface> get_interface() { return this->interface_; };

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

class PluginPythonLoader : public PluginLoader
{
public:
    PluginPythonLoader(const std::string &package_name);
    virtual ~PluginPythonLoader(){};

public:
    virtual bool load();
    virtual bool activate();
    virtual bool deactivate();
    virtual std::shared_ptr<SSRPluginInterface> get_interface() { return this->interface_; };

private:
    // 包名
    std::string package_name_;
    // 是否已经激活
    bool is_activate_;
    std::shared_ptr<SSRPluginInterface> interface_;
};

}  // namespace Daemon
}  // namespace KS