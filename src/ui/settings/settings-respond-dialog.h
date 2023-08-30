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
 * Author:     yuanxing <yuanxing@kylinos.com.cn>
 */

#pragma once

#include <QLabel>
#include "src/ui/common/titlebar-window.h"
namespace KS
{
class SettingsRespondDialog : public TitlebarWindow
{
    Q_OBJECT
public:
    SettingsRespondDialog(QWidget* parent = nullptr);
    virtual ~SettingsRespondDialog();
    void setMessage(const QString& text);

protected:
    void closeEvent(QCloseEvent* event);

private:
    void initUI();

signals:
    void accepted();
    void rejected();

private:
    QLabel* m_message;
    bool m_isAccepted;
};
}  // namespace KS
