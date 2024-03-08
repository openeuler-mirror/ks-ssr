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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */
#pragma once

#include "src/ui/common/page.h"

class ToolBoxDbusProxy;

namespace Ui
{
class FileSignPage;
}  // namespace Ui

namespace KS
{
namespace ToolBox
{
class ModifySecurityContext;
class AddUserDialog;

class FileSign : public Page
{
    Q_OBJECT
public:
    FileSign(QWidget* parent = nullptr);
    virtual ~FileSign();
    virtual QString getNavigationUID() override;
    virtual QString getSidebarUID() override;
    virtual QString getSidebarIcon() override;
    virtual QString getAccountRoleName() override;
    void updateTableData(const QStringList& fileList);

private:
    void initConnection();

private slots:
    void openFileDialog(bool);
    void popModifySecurityContext(const QModelIndex& index);
    void acceptedSecurityContext();

private:
    Ui::FileSignPage* m_ui;
    ToolBoxDbusProxy* m_dbusProxy;
    ModifySecurityContext* m_modifySecurityContext;
    AddUserDialog* m_inputUsers;
};
}  // namespace ToolBox
}  // namespace KS
