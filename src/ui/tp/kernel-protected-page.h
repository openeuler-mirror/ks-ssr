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

#include <QTimer>
#include <QWidget>
#include "src/ui/common/page.h"
#include "src/ui/tp/kernel-protected-table.h"

namespace Ui
{
class KernelProtectedPage;
}

namespace KS
{
namespace TP
{
class KernelProtectedPage : public Page
{
    Q_OBJECT
public:
    KernelProtectedPage(QWidget *parent = nullptr);
    ~KernelProtectedPage();

    QString getNavigationUID() override;
    QString getSidebarUID() override;
    QString getSidebarIcon() override;
    QString getAccountRoleName() override;

private:
    void updateTips(int total);
    bool isExistSelectedItem();

private Q_SLOTS:
    void setSearchText(const QString &text);
    void addKernelFile(bool checked);
    void updateKernelList(bool checked);
    void recertification(bool checked);
    void popDeleteNotify(bool checked);
    void removeKernelFiles();
    void prohibitUnloading(bool status, const QString &path);
    void updateRefreshIcon();

private:
    Ui::KernelProtectedPage *m_ui;

    KSSDbusProxy *m_dbusProxy;

    QTimer *m_refreshTimer;
};
}  // namespace TP
}  // namespace KS
