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
#include <QJsonValue>
#include <QWidget>
#include "br-protocol.hxx"

class QHBoxLayout;
class QLineEdit;

namespace KS
{
class SpinBox;
class ComboBox;

namespace BR
{
class ArgHandle : public QWidget
{
    Q_OBJECT

public:
    explicit ArgHandle(QWidget *parent = 0,
                       const QString &itemKey = "",
                       const QString &argName = "",
                       const QString &label = "",
                       const QString &valueLimits = "",
                       const QString &inputExample = "",
                       const QJsonValue &jsonValue = "",
                       KS::Protocol::WidgetType::Value widgetType = KS::Protocol::WidgetType::Value::DEFAULT,
                       const QString &note = "");
    virtual ~ArgHandle(){};

    void setValue(const QJsonValue &jsonValue);
    QString getCategoryName();
    QString getArgName();
    QString getLabel();
    int getValueLimits();
    bool valueCheck();
    KS::Protocol::WidgetType::Value getWidgetType();

private:
    void init(const QJsonValue &jsonValue, KS::Protocol::WidgetType::Value widgetType);
    void initText(const QJsonValue &jsonValue);
    void initSwitch(const QJsonValue &jsonValue);
    void initInteger(const QJsonValue &jsonValue);
    void initDefault(const QJsonValue &jsonValue);
    QHBoxLayout *buildLabelLayout();
    void confirmType(const QJsonValue &jsonValue);

signals:
    void valueChanged(const QString &categoryName, const QString &argName, const QString &argValue, KS::Protocol::WidgetType::Value widgetType);
    void reset(const QString &categoryName, const QString &argName);

private slots:
    // 下拉框参数改变处理
    void changedBoolArgs(int index);
    // 文本框类型参数改变处理
    void changedStringArgs(const QString &str);
    // 数字输入框参数改变处理
    void changedIntArgs(int value);
    void argReset();

private:
    QString m_itemKey;
    QString m_argName;
    QString m_label;
    QString m_valueLimits;
    QString m_inputExample;
    QString m_note;
    KS::Protocol::WidgetType::Value m_widgetType;
    // 文本输入框
    QLineEdit *m_lineEdit;
    // 整形输入框
    SpinBox *m_spinBox;
    // 下拉框
    ComboBox *m_comboBox;
};

}  // namespace BR
}  // namespace KS
