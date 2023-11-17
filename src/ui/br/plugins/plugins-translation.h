#pragma once

#include <QCoreApplication>

namespace KS
{
namespace BR
{
namespace Plugins
{
#define PLUGINS_TRANSLATION_KEY "python"


class PluginsTranslation
{
public:
    static void globalInit(){
        m_instance = new PluginsTranslation();
    };
    static void globalDeinit(){
        if (m_instance)
        {
            delete m_instance;
        }
    };

    static PluginsTranslation *instance() { return m_instance; };
private:
    PluginsTranslation()
    {
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Device busy, please pop up!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Please contact the admin."));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Unable to stop service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Abnormal service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Please close SELinux and use it!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "No such file or directory."));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Failed to execute command. Please check the log information for details."));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "UsePAM is not recommended to be closed,\nwhich will cause many problems!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Unable to stop firewalld service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Unable to stop bluetooth service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Unable to stop cups service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Unable to stop avahi service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Unable to stop rpcbind service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Unable to stop smb service!"));
        Q_ASSERT(QT_TRANSLATE_NOOP_UTF8(PLUGINS_TRANSLATION_KEY, "Abnormal service! Please check the log information for details."));

    };
    ~PluginsTranslation(){};

private:
    static PluginsTranslation *m_instance;
};

PluginsTranslation *PluginsTranslation::m_instance = nullptr;
}  // namespace Plugins
}  // namespace BR
}  // namespace KS
