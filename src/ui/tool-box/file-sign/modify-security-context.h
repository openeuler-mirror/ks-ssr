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

#include <QCloseEvent>
#include <QWidget>
#include "include/ssr-i.h"
#include "src/ui/common/window/titlebar-window.h"

namespace Ui
{
class ModifySecurityContext;
}

namespace KS
{
namespace ToolBox
{
class ModifySecurityContext : public TitlebarWindow
{
    Q_OBJECT

public:
    ModifySecurityContext(QWidget *parent = nullptr);
    ~ModifySecurityContext();

    void setSecurityContext(const QString &text);
    QString getSecurityContext() const;

    void setIntegrityLabel(const QString &text);
    QString getIntegrityLabel() const;

    void setFilePath(const QString &path);
    QString getFilePath() const;
protected:
    void closeEvent(QCloseEvent *event);

private:
    void initUI();

signals:
    void accepted();
    void closed();

private:
    Ui::ModifySecurityContext *m_ui;

    QString m_filePath;
};
}  // namespace ToolBox
}  // namespace KS
