/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
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

#include "private-box-plugin.h"
#include "box-manager.h"

namespace KS
{
namespace PrivateBox
{
bool PrivateBoxPlugin::isAvailable()
{
    return true;
}

void PrivateBoxPlugin::activate()
{
    BoxManager::globalInit(this);
}

void PrivateBoxPlugin::deactivate()
{
    BoxManager::globalDeinit();
}
}  // namespace PrivateBox

}  // namespace KS
