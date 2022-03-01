/**
 * @file          /ks-ssr-manager/src/daemon/python/plugin-python.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

struct _object;
typedef struct _object PyObject;

namespace KS
{
namespace Daemon
{
class ReinforcementPython : public SSRReinforcementInterface
{
public:
    ReinforcementPython(PyObject *module,
                        const std::string &class_name);
    virtual ~ReinforcementPython();

    virtual bool get(std::string &args, std::string &error) override;
    virtual bool set(const std::string &args, std::string &error) override;

    bool is_valid() { return valid_; };

private:
    bool check_call_result(PyObject *py_retval, const std::string &function_name, std::string &error);

private:
    PyObject *module_;
    std::string module_fullname_;
    std::string class_name_;
    // python类
    PyObject *class_;
    // python对象
    PyObject *class_instance_;

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
    void add_reinforcement(const std::string &package_name,
                           const std::string &module_name,
                           const std::string &reinforcement_name,
                           const std::string &function_prefix);

    PyObject *get_reinforcement_module(const std::string &module_fullname) { return MapHelper::get_value(this->reinforcements_modules_, module_fullname); };

private:
    PyObject *module_;

    std::map<std::string, std::shared_ptr<SSRReinforcementInterface>> reinforcements_;

    std::map<std::string, PyObject *> reinforcements_modules_;
};
}  // namespace Daemon
}  // namespace KS
