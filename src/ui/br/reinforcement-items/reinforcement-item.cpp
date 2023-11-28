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

#include "reinforcement-item.h"
namespace KS
{
namespace BR
{
ReinforcementItem::ReinforcementItem()
{
}

void ReinforcementItem::setName(const QString &name)
{
    m_name = name;
}

void ReinforcementItem::setLabel(const QString &label)
{
    m_label = label;
}

void ReinforcementItem::setDescription(const QString &description)
{
    m_description = description;
}

void ReinforcementItem::setArg(const QString &argName,
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

void ReinforcementItem::setCategoryName(const QString &categoryName)
{
    m_categoryName = categoryName;
}

void ReinforcementItem::setLayout(const QJsonObject &layout)
{
    m_layout = layout;
}

void ReinforcementItem::setState(int state)
{
    m_state = state;
}

void ReinforcementItem::setScanState(int state)
{
    m_scanState = state;
}

void ReinforcementItem::setFastenState(int state)
{
    m_fastenState = state;
}

void ReinforcementItem::setCheckStatus(bool checkStatus)
{
    m_checkStatus = checkStatus;
}

void ReinforcementItem::setErrorMessage(const QString &error)
{
    m_errorMessage = error;
}

QString ReinforcementItem::getName()
{
    return m_name;
}

QString ReinforcementItem::getLabel()
{
    return m_label;
}

QString ReinforcementItem::getCategoryName()
{
    return m_categoryName;
}

QString ReinforcementItem::getErrorMessage()
{
    return m_errorMessage;
}

QJsonObject ReinforcementItem::getLayout()
{
    return m_layout;
}

int ReinforcementItem::getState()
{
    int state = m_state;
    return state;
}

int ReinforcementItem::getScanState()
{
    return m_scanState;
}

int ReinforcementItem::getFastenState()
{
    return m_fastenState;
}

bool ReinforcementItem::getCheckStatus()
{
    return m_checkStatus;
}

QString ReinforcementItem::getDescription()
{
    return m_description;
}

std::vector<CategoryArgs *> ReinforcementItem::getArgs()
{
    return m_args;
}

CategoryArgs *ReinforcementItem::find(const QString &argName)
{
    for (auto arg : m_args)
    {
        if (arg->name == argName)
            return arg;
    }
    return NULL;
}
}  // namespace BR
}  // namespace KS
