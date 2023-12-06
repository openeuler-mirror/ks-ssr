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

#include <iostream>
#include <vector>
#define INIT_LIBNOTIFY_ERROR -1
#define MAX_NOTIFY_NUMBER 5

struct _NotifyNotification;
typedef struct _NotifyNotification NotifyNotification;

namespace Notify
{
#define NOTIFY_INFO(message) NotificationWrapper::getInstance()->info(message);

#define NOTIFY_WARN(message) NotificationWrapper::getInstance()->warn(message);

#define NOTIFY_ERROR(message) NotificationWrapper::getInstance()->error(message);

class NotificationWrapper
{
public:
    NotificationWrapper(std::string m_appName);
    virtual ~NotificationWrapper();

    static NotificationWrapper *getInstance();

    static void globalInit(std::string m_appName);

    static void globalDeinit();
    // 普通消息
    void info(const char *message);

    // 告警消息
    void warn(const char *message);

    // 错误消息
    void error(const char *message);

private:
    void notifySend(const char *msg, const char *icon);
    // 超过5条将开始的一条销毁
    void checkNotifiesAndDelete();

private:
    static NotificationWrapper *m_instance;
    // 存储消息列表 至多5条
    std::vector<NotifyNotification *> m_notifies;

    std::string m_appName;
};
}  // namespace Notify
