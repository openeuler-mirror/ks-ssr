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
#include "spinbox.h"
#include <QKeyEvent>

namespace KS
{
SpinBox::SpinBox(QWidget *parent)
    : QSpinBox(parent)
{
}

SpinBox::~SpinBox()
{
}

void SpinBox::wheelEvent(QWheelEvent *event)
{
    Q_ASSERT(event);
    return;
}

void SpinBox::keyPressEvent(QKeyEvent *event)
{
    if (event->key() != Qt::Key_Comma)
    {
        QSpinBox::keyPressEvent(event);
        return;
    }

    event->ignore();
}
}  // namespace KS
