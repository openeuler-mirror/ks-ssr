/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     wangxiaoqing <wangxiaoqing@kylinos.com.cn>
 */

#include "src/daemon/device/device-interface.h"
#include <config.h>
#include <qt5-log-i.h>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "ksc-i.h"
#include "ksc-marcos.h"

namespace KS
{

DeviceInterface::DeviceInterface(QObject* parent)
    : QObject{parent}
{
    m_ruleManager = DeviceRuleManager::instance();
}

QString DeviceInterface::getInterfaces()
{
    QJsonDocument jsonDoc;
    QJsonArray jsonArray;
    int type;

    for (type = INTERFACE_TYPE_USB; type < INTERFACE_TYPE_UNKNOWN; type++)
    {
        QJsonObject jsonObj{
            {KSC_DI_JK_TYPE, type},
            {KSC_DI_JK_ENABLE, m_ruleManager->isIFCEnable(type)}};

        jsonArray.append(jsonObj);
    }

    jsonDoc.setArray(jsonArray);

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

QString DeviceInterface::getInterface(int type)
{
    QJsonDocument jsonDoc;
    int interfaceType;

    for (interfaceType = INTERFACE_TYPE_USB; interfaceType < INTERFACE_TYPE_UNKNOWN; interfaceType++)
    {
        if (type == interfaceType)
        {
            QJsonObject jsonObj{
                {KSC_DI_JK_TYPE, interfaceType},
                {KSC_DI_JK_ENABLE, m_ruleManager->isIFCEnable(interfaceType)}};
            jsonDoc.setObject(jsonObj);
            break;
        }
    }

    return QString(jsonDoc.toJson(QJsonDocument::Compact));
}

bool DeviceInterface::setInterfaceEnable(int type,
                                         bool enable)
{
    return m_ruleManager->setIFCEnable(type, enable);
}

bool DeviceInterface::verifyInterface(int type)
{
    RETURN_VAL_IF_FALSE(type >= INTERFACE_TYPE_USB &&
                            type < INTERFACE_TYPE_UNKNOWN,
                        false)
    return true;
}

}  // namespace KS
