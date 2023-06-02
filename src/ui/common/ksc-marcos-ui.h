/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
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
#ifndef KSCMARCOSUI_H
#define KSCMARCOSUI_H

#include "include/ksc-marcos.h"
#include "src/ui/common/message-dialog.h"

// MessageDialog marcos
// MessageDialog设置了关闭即销毁属性
#define POPUP_MESSAGE_DIALOG(message, parent)                                         \
    {                                                                                 \
        auto messageDialog = new MessageDialog(parent);                               \
        messageDialog->setMessage(message);                                           \
        int x = window()->x() + window()->width() / 4 + messageDialog->width() / 4;   \
        int y = window()->y() + window()->height() / 4 + messageDialog->height() / 4; \
        messageDialog->move(x, y);                                                    \
        messageDialog->show();                                                        \
    \ 
}

#define CHECK_ERROR_FOR_DBUS_REPLY(reply, successMessage, parent) \
    {                                                             \
        reply.waitForFinished();                                  \
        if (reply.isError())                                      \
        {                                                         \
            POPUP_MESSAGE_DIALOG(reply.error().message(), parent) \
        }                                                         \
        if (!successMessage.isEmpty())                            \
        {                                                         \
            POPUP_MESSAGE_DIALOG(successMessage, parent)          \
        }                                                         \
    }

#define POPUP_MESSAGE_DIALOG_RETURN(message, parent)                                  \
    {                                                                                 \
        auto messageDialog = new MessageDialog(parent);                               \
        messageDialog->setMessage(message);                                           \
        int x = window()->x() + window()->width() / 4 + messageDialog->width() / 4;   \
        int y = window()->y() + window()->height() / 4 + messageDialog->height() / 4; \
        messageDialog->move(x, y);                                                    \
        messageDialog->show();                                                        \
        return;                                                                       \
    }
// successMessage会在执行成功时弹出，为QString("")时不弹窗
#define CHECK_ERROR_FOR_DBUS_REPLY_RETURN(reply, successMessage, parent)  \
    {                                                                     \
        reply.waitForFinished();                                          \
        if (reply.isError())                                              \
        {                                                                 \
            POPUP_MESSAGE_DIALOG_RETURN(reply.error().message(), parent); \
        }                                                                 \
        if (!successMessage.isEmpty())                                    \
        {                                                                 \
            POPUP_MESSAGE_DIALOG_RETURN(successMessage, parent);          \
        }                                                                 \
        return;                                                           \
    }

#define POPUP_MESSAGE_DIALOG_RETURN_VAL(message, val, parent)                         \
    {                                                                                 \
        auto messageDialog = new MessageDialog(parent);                               \
        messageDialog->setMessage(message);                                           \
        int x = window()->x() + window()->width() / 4 + messageDialog->width() / 4;   \
        int y = window()->y() + window()->height() / 4 + messageDialog->height() / 4; \
        messageDialog->move(x, y);                                                    \
        messageDialog->show();                                                        \
        return val;                                                                   \
    }

#define CHECK_ERROR_FOR_DBUS_REPLY_RETURN_VAL(reply, successMessage, val, parent) \
    {                                                                             \
        reply.waitForFinished();                                                  \
        if (reply.isError())                                                      \
        {                                                                         \
            POPUP_MESSAGE_DIALOG_RETURN_VAL(reply.error().message(), val, parent) \
        }                                                                         \
        if (!successMessage.isEmpty())                                            \
        {                                                                         \
            POPUP_MESSAGE_DIALOG_RETURN_VAL(successMessage, val, parent)          \
        }                                                                         \
        return val;                                                               \
    }

#endif  // KSCMARCOSUI_H
