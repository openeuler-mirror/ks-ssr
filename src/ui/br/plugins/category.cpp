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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */

#include "category.h"
namespace KS
{
namespace BR
{
namespace Plugins
{
Category::Category()
{
}

void Category::setName(const QString &name)
{
    m_name = name;
}

void Category::setLabel(const QString &label)
{
    m_label = label;
}

void Category::setDescription(const QString &description)
{
    m_description = description;
}

void Category::setArg(const QString &argName,
                      const QJsonValue &jsonValue,
                      KS::Protocol::WidgetType::Value widgetType,
                      const QString &valueLimits,
                      const QString &inputExample,
                      const QString &label,
                      const QString &note)
{
    auto arg = new CategoryArgs();
    arg->name = argName;
    arg->jsonValue = jsonValue;
    arg->widgetType = widgetType;
    arg->valueLimits = valueLimits;
    arg->inputExample = inputExample;
    arg->label = label;
    arg->note = note;
    m_args.push_back(arg);
}

void Category::setCategoryName(const QString &categoryName)
{
    m_categoryName = categoryName;
}

void Category::setLayout(const QJsonObject &layout)
{
    m_layout = layout;
}

void Category::setState(int state)
{
    m_state = state;
}

void Category::setScanState(int state)
{
    m_scanState = state;
}

void Category::setFastenState(int state)
{
    m_fastenState = state;
}

void Category::setCheckStatus(bool checkStatus)
{
    m_checkStatus = checkStatus;
}

void Category::setErrorMessage(const QString &error)
{
    m_errorMessage = error;
}

QString Category::getName()
{
    return m_name;
}

QString Category::getLabel()
{
    return m_label;
}

QString Category::getCategoryName()
{
    return m_categoryName;
}

QString Category::getErrorMessage()
{
    return m_errorMessage;
}

QJsonObject Category::getLayout()
{
    return m_layout;
}

int Category::getState()
{
    int state = m_state;
    return state;
}

int Category::getScanState()
{
    return m_scanState;
}

int Category::getFastenState()
{
    return m_fastenState;
}

bool Category::getCheckStatus()
{
    return m_checkStatus;
}

QString Category::getDescription()
{
    return m_description;
}

std::vector<CategoryArgs *> Category::getArgs()
{
    return m_args;
}

CategoryArgs *Category::find(const QString &argName)
{
    for (auto arg : m_args)
    {
        if (arg->name == argName)
            return arg;
    }
    return NULL;
}
}  // namespace Plugins
}  // namespace BR
}  // namespace KS
