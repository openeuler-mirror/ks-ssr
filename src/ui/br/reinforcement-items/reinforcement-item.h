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
#pragma once

#include <QJsonObject>
#include <QStandardItem>
#include "br-protocol.hxx"

namespace KS
{
namespace BR
{
struct CategoryArgs
{
    QString name;
    QJsonValue jsonValue;
    KS::Protocol::WidgetType::Value widgetType;
    QString valueLimits;
    QString inputExample;
    QString label;
    QString description;
    QString note;
    // 构造函数
    CategoryArgs(const QString &n = "",
                 const QString &l = "",
                 const QString &r = "",
                 const QString &des = "",
                 const QString &e = "",
                 const QString &t = "")
        : name(n),
          jsonValue(""),
          widgetType(KS::Protocol::WidgetType::Value::DEFAULT),
          valueLimits(r),
          inputExample(e),
          label(l),
          description(des),
          note(t)
    {
    }
};

class ReinforcementItem
{
public:
    ReinforcementItem();
    virtual ~ReinforcementItem(){};

    void setName(const QString &name);
    void setLabel(const QString &label);
    void setDescription(const QString &description);
    void setArg(const QString &argName,
                const QJsonValue &jsonValue,
                KS::Protocol::WidgetType::Value widgetType,
                const QString &valueLimits,
                const QString &inputExample,
                const QString &label,
                const QString &note);
    void setLayout(const QJsonObject &layout);
    void setState(int state);
    void setScanState(int state);
    void setFastenState(int state);
    void setCheckStatus(bool checkStatus);
    void setCategoryName(const QString &categoryName);
    void setErrorMessage(const QString &error);

    QString getName();
    QString getLabel();
    QString getDescription();
    QString getCategoryName();
    QString getErrorMessage();
    std::vector<CategoryArgs *> getArgs();
    QJsonObject getLayout();
    int getState();
    int getScanState();
    int getFastenState();
    bool getCheckStatus();
    CategoryArgs *find(const QString &argName);

public:
    bool changeFlag = false;

private:
    QString m_categoryName;
    QString m_label;
    QString m_description;
    QString m_name;
    std::vector<CategoryArgs *> m_args;
    QJsonObject m_layout;
    int m_state = 0;
    int m_scanState = 4;
    int m_fastenState = 64;
    bool m_checkStatus = false;
    QString m_errorMessage = "";
};

}  // namespace BR
}  // namespace KS
