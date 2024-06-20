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
 * Author:     wangyucheng <wangyucheng@kylinsec.com.cn>
 */

#pragma once

#include <br-plugin-i.h>
#include <QMap>
#include <QSharedPointer>
#include "lib/base/base.h"

struct _object;
typedef struct _object PyObject;

namespace KS
{
namespace BR
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
        return m_valid;
    };

private:
    bool checkCallResult(PyObject *pyRetval, const QString &functionName, QString &error);

private:
    PyObject *m_module;
    QString m_moduleFullname;
    QString m_className;
    // python类
    PyObject *m_class;
    // python对象
    PyObject *m_classInstance;
    bool m_valid;
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
        return MapHelper::getValue(this->m_reinforcements, name);
    };

private:
    void addReinforcement(const QString &packageName,
                          const QString &moduleName,
                          const QString &reinforcementName,
                          const QString &functionPrefix);

    PyObject *getReinforcementModule(const QString &moduleFullname)
    {
        return MapHelper::getValue(this->m_reinforcementsModules, moduleFullname);
    };

private:
    PyObject *m_module;
    QMap<QString, QSharedPointer<BRReinforcementInterface>> m_reinforcements;
    QMap<QString, PyObject *> m_reinforcementsModules;
};
}  // namespace BR
}  // namespace KS
