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
#include "arghandle.h"
#include "src/ui/common/window/titlebar-window.h"

namespace Ui
{
class ReinforcementArgsDialog;
}

namespace KS
{
namespace BR
{
#define LINE_HEIGHT 100
#define MIN_HEIGHT 150
#define MAX_HEIGHT 450

class ReinforcementArgsDialog : public TitlebarWindow
{
    Q_OBJECT

public:
    explicit ReinforcementArgsDialog(QWidget *parent = 0);
    virtual ~ReinforcementArgsDialog();
    virtual QSize sizeHint() const override;
    // 添加设置加固项参数ui
    void addLine(const QString &name,
                 const QString &argName,
                 const QString &label,
                 const QString &valueLimits,
                 const QString &inputExample,
                 const QJsonValue &jsonValue,
                 KS::Protocol::WidgetType::Value widgetType,
                 const QString &note);
    int getHeight();
    void clear();
    void setValue(const QJsonValue &jsonValue);

protected:
    void closeEvent(QCloseEvent *) override;

signals:
    void argError(const QString &str);
    void reseted(const QString &categoryName, const QString &argName);
    void closed();
    void okClicked();
    void valueChanged(const QString &name,
                      const QString &argName,
                      const QString &value,
                      KS::Protocol::WidgetType::Value widgetType);

private slots:
    void setArgs();
    void argReset();

private:
    void init();

private:
    // json value list
    QStringList m_args;
    // 加固项参数 每一个ArgHandle表示一个加固项参数
    QList<ArgHandle *> m_argHandle;
    // 当前设置的加固项参数
    ArgHandle *m_currentArgHandle;

    int m_height;

    Ui::ReinforcementArgsDialog *m_ui;
};
}  // namespace BR
}  // namespace KS
