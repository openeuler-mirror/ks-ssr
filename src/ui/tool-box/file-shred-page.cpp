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

#include "file-shred-page.h"
#include "include/ssr-i.h"

#define FILE_SHRED_ICON_NAME "/images/file-shred"

namespace KS
{
namespace ToolBox
{
FileShred::FileShred(QWidget* parent)
    : Page(parent)
{
}

FileShred::~FileShred()
{
}

QString FileShred::getNavigationUID()
{
    return tr("Tool Box");
}

QString FileShred::getSidebarUID()
{
    return tr("File Shred");
}

QString FileShred::getSidebarIcon()
{
    return ":" FILE_SHRED_ICON_NAME;
}

QString FileShred::getAccountRoleName()
{
    return SSR_ACCOUNT_NAME_SECADM;
}

}  // namespace ToolBox
}  // namespace KS