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

#include "password-event-filter.h"
#include <QKeyEvent>
#include "common/ssr-marcos-ui.h"

namespace KS
{
PasswordEventFilter::PasswordEventFilter(QObject *parent) : QObject(parent)
{

}

bool PasswordEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        auto keyEvent = dynamic_cast<QKeyEvent *>(event);
        RETURN_VAL_IF_TRUE(keyEvent->matches(QKeySequence::Copy) || keyEvent->matches(QKeySequence::Paste) || keyEvent->matches(QKeySequence::Cut), true);
    }

    return QObject::eventFilter(watched, event);
}

}  // namespace KS
