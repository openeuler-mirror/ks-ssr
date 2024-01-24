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

#include "src/ui/private-box/boxs.h"
#include "src/ui/private-box/flow-layout.h"

namespace KS
{
namespace PrivateBox
{
Boxs::Boxs(QWidget *parent)
    : QWidget(parent)
{
    setLayout(new FlowLayout(this, 0, 16, 16));
}

void Boxs::addBox(Box *box)
{
    auto layout = qobject_cast<FlowLayout *>(this->layout());
    layout->addWidget(box);
}

void Boxs::removeBox(Box *box)
{
    box->deleteLater();
    box = nullptr;
}
}  // namespace PrivateBox
}  // namespace KS