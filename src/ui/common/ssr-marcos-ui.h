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

#include "include/ssr-marcos.h"
#include "src/ui/common/message-dialog.h"

// MessageDialog marcos
// MessageDialog设置了关闭即销毁属性
#define POPUP_MESSAGE_DIALOG(message)                                                 \
    {                                                                                 \
        auto messageDialog = new MessageDialog(this->window());                       \
        messageDialog->setMessage(message);                                           \
        int x = window()->x() + window()->width() / 2 - messageDialog->width() / 2;   \
        int y = window()->y() + window()->height() / 2 - messageDialog->height() / 2; \
        messageDialog->move(x, y);                                                    \
        messageDialog->show();                                                        \
    }

#define CHECK_ERROR_FOR_DBUS_REPLY(reply)                 \
    {                                                     \
        reply.waitForFinished();                          \
        if (reply.isError())                              \
        {                                                 \
            POPUP_MESSAGE_DIALOG(reply.error().message()) \
        }                                                 \
    }
