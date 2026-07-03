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
 * Author:     tangjie02 <tangjie02@kylinos.com.cn>
 */

#pragma once

#include <QWidget>
#include "src/ui/common/page.h"

class KSSDbusProxy;

namespace Ui
{
class FileProtection;
}  // namespace Ui

namespace KS
{
namespace FP
{
class FileProtection : public Page
{
    Q_OBJECT
public:
    FileProtection(QWidget *parent = nullptr);
    virtual ~FileProtection();

    QString getNavigationUID() override;
    QString getSidebarUID() override;
    QString getSidebarIcon() override;
    int getSelinuxType() override;

private:
    bool checkTrustedLoadFinied(bool initialized);
    bool isExistSelectedItem();

private Q_SLOTS:
    void searchTextChanged(const QString &text);
    void addProtectedFile(bool checked);
    void popDeleteNotify(bool checked);
    void removeProtectedFiles();
    void updateTips(int total);

private:
    Ui::FileProtection *m_ui;

    KSSDbusProxy *m_fileProtectedProxy;
};
}  // namespace FP
}  // namespace KS
