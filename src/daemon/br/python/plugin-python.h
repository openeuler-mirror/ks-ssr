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

#include <QMap>
#include <QSharedPointer>
#include "include/br-plugin-i.h"
#include "lib/base/base.h"

struct _object;
typedef struct _object PyObject;

namespace KS
{
namespace BRDaemon
{
class ReinforcementPython : public BRReinforcementInterface
{
public:
    ReinforcementPython(PyObject *module,
                        const QString &class_name);
    virtual ~ReinforcementPython();

    virtual bool get(QString &args, QString &error);
    virtual bool set(const QString &args, QString &error);

    bool isValid()
    {
        return valid_;
    };

private:
    bool check_call_result(PyObject *py_retval, const QString &function_name, QString &error);

private:
    PyObject *module_;
    QString module_fullname_;
    QString class_name_;
    // python类
    PyObject *class_;
    // python对象
    PyObject *class_instance_;

    bool valid_;
};

class PluginPython : public BRPluginInterface
{
public:
    PluginPython(PyObject *module);
    virtual ~PluginPython();

    virtual void activate() override;
    virtual void deactivate() override;
    void clean();

    virtual QSharedPointer<BRReinforcementInterface> getReinforcement(const QString &name) override
    {
        return MapHelper::getValue(this->reinforcements_, name);
    };

private:
    void add_reinforcement(const QString &package_name,
                           const QString &module_name,
                           const QString &reinforcement_name,
                           const QString &function_prefix);

    PyObject *get_reinforcement_module(const QString &module_fullname)
    {
        return MapHelper::getValue(this->reinforcements_modules_, module_fullname);
    };

private:
    PyObject *module_;

    QMap<QString, QSharedPointer<BRReinforcementInterface>> reinforcements_;

    QMap<QString, PyObject *> reinforcements_modules_;
};
}  // namespace BRDaemon
}  // namespace KS
