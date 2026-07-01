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

#include <qt5-log-i.h>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusContext>
#include <QDBusMessage>

class DBusHelper
{
public:
    inline static QString getCallerUniqueName(const QDBusContext* _this)
    {
        if (_this == nullptr || !_this->calledFromDBus())
        {
            return QString();
        }
        return _this->message().service();
    }

    inline static pid_t getCallerPid(const QDBusContext* _this)
    {
        if (_this == nullptr || !_this->calledFromDBus())
        {
            return -1;
        }
        auto dbusConn = _this->connection();
        auto dbusMsg = _this->message();

        return dbusConn.interface()->servicePid(dbusMsg.service());
    }
};