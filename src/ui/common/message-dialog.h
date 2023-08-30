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

#include <QVBoxLayout>
#include "src/ui/common/titlebar-window.h"

namespace KS
{
// 自定义消息对话框，统一提示信息对话框
class MessageDialog : public TitlebarWindow
{
    Q_OBJECT
public:
    MessageDialog(QWidget *parent = nullptr);
    virtual ~MessageDialog();

    void setMessage(const QString &message);

private:
    void initUI();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QVBoxLayout *m_contentLayout;
};
}  // namespace KS
