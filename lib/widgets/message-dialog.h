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
 * Author:     chendingjian <chendingjian@kylinsec.com.cn>
 */

#pragma once

#include "window/titlebar-window.h"

class QLabel;
namespace KS
{
// 自定义消息对话框，统一提示信息对话框
class MessageDialog : public TitlebarWindow
{
    Q_OBJECT
public:
    MessageDialog(QWidget *parent = nullptr, bool canGetResult = false);
    virtual ~MessageDialog();

    void setMessage(const QString &message);
    bool exec();

private:
    void initUI(bool canGetResult);

protected:
    void paintEvent(QPaintEvent *event);

signals:
    void finished();

private:
    QLabel *m_messageLabel;

    bool m_result;
};
}  // namespace KS
