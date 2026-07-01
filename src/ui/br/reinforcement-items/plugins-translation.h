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

#include <QCoreApplication>

namespace KS
{
namespace BR
{
class PluginsTranslation
{
public:
    static void globalInit()
    {
        m_instance = new PluginsTranslation();
    };
    static void globalDeinit()
    {
        if (m_instance)
        {
            delete m_instance;
        }
    };

    static PluginsTranslation *instance()
    {
        return m_instance;
    };

private:
    PluginsTranslation()
    {
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Device busy, please pop up!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Please contact the admin."));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Abnormal service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Please close SELinux and use it!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "No such file or directory."));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Failed to execute command. Please check the log information for details."));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "UsePAM is not recommended to be closed,\nwhich will cause many problems!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop firewalld service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop bluetooth service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop cups service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop avahi service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop rpcbind service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Unable to stop smb service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "sshd.services is not running!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8("python", "Abnormal service! Please check the log information for details."));
    };
    ~PluginsTranslation(){};

private:
    static PluginsTranslation *m_instance;
};

PluginsTranslation *PluginsTranslation::m_instance = nullptr;
}  // namespace BR
}  // namespace KS
