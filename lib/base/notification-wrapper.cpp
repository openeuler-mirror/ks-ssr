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
#include "notification-wrapper.h"
#include <libnotify/notify.h>
#include <memory>
#include "config.h"

using std::cerr;
using std::endl;

namespace Notify
{
#define NOTIFY_TIMEOUT (10 * 1000)
#define NOTIFY_ICON_INFO "gtk-dialog-info"
#define NOTIFY_ICON_WARN "gtk-dialog-warning"
#define NOTIFY_ICON_ERROR "gtk-dialog-error"

#define LIBNOTIFY_CHECK_VERSION(major, minor) \
    ((LIBNOTIFY_MAJOR > major) ||             \
     (LIBNOTIFY_MAJOR == major && LIBNOTIFY_MINOR >= minor))

NotificationWrapper::NotificationWrapper(std::string app_name)
    : m_notifies({}),
      m_appName(app_name)
{
    // TODO:notify 目前为gtk实现，改为QT实现后此处逻辑需要修改为KLOG_WARRING
    if (!notify_init(app_name.c_str()))
    {
        cerr << "Failed to init libnotify" << endl;
    }
}

NotificationWrapper::~NotificationWrapper()
{
    for (auto i = 0; i < m_notifies.size() ; i++)
    {
        if (m_notifies.at(i))
        {
            g_object_unref(m_notifies.at(i));
        }
    }
    m_notifies.clear();
    notify_uninit();
}

NotificationWrapper *NotificationWrapper::m_instance = nullptr;

NotificationWrapper *NotificationWrapper::getInstance()
{
    return m_instance;
}

void NotificationWrapper::globalInit(std::string app_name)
{
    if (!m_instance)
    {
        m_instance = new NotificationWrapper(app_name);
    }
}

void NotificationWrapper::globalDeinit()
{
    delete m_instance;
}

void NotificationWrapper::info(const char *message)
{
    this->notifySend(message, NOTIFY_ICON_INFO);
}

void NotificationWrapper::warn(const char *message)
{
    this->notifySend(message, NOTIFY_ICON_WARN);
}

void NotificationWrapper::error(const char *message)
{
    this->notifySend(message, NOTIFY_ICON_ERROR);
}

void NotificationWrapper::notifySend(const char *msg, const char *icon)
{
#if LIBNOTIFY_CHECK_VERSION(0, 7)
    auto notify = notify_notification_new(m_appName.c_str(), msg, icon);
#else
    auto notify = notify_notification_new(m_appName.c_str(), msg, icon, NULL);
#endif
    m_notifies.push_back(notify);
    notify_notification_set_timeout(notify, NOTIFY_TIMEOUT);
    notify_notification_show(notify, NULL);
    checkNotifiesAndDelete();
}

void NotificationWrapper::checkNotifiesAndDelete()
{
    if (m_notifies.size() <= MAX_NOTIFY_NUMBER)
    {
        return;
    }

    if (m_notifies.at(0))
    {
        g_object_unref(m_notifies.at(0));
    }
    m_notifies.erase(m_notifies.begin());
}

}  // namespace Notify
