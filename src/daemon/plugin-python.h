/**
 * @file          /kiran-ssr-manager/src/daemon/plugin-python.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

struct _object;
typedef struct _object PyObject;

namespace Kiran
{
namespace Daemon
{
class ReinforcementPython : public SSRReinforcementInterface
{
public:
    ReinforcementPython(const std::string &package_name, const std::string &module_name);
    virtual ~ReinforcementPython();

    virtual bool get(std::string &args, SSRErrorCode &error_code) override;
    virtual bool set(const std::string &args, SSRErrorCode &error_code) override;

    bool is_valid() { return valid_; };

private:
    std::string package_name_;
    std::string module_name_;

    PyObject *module_;
    PyObject *get_method_;
    PyObject *set_method_;

    bool valid_;
};

class PluginPython : public SSRPluginInterface
{
public:
    PluginPython(PyObject *module);
    virtual ~PluginPython();

    virtual void activate() override;
    virtual void deactivate() override;

    virtual std::shared_ptr<SSRReinforcementInterface> get_reinforcement(const std::string &name) override { return MapHelper::get_value(this->reinforcements_, name); };

private:
    PyObject *module_;

    std::map<std::string, std::shared_ptr<SSRReinforcementInterface>> reinforcements_;
};
}  // namespace Daemon
}  // namespace Kiran
