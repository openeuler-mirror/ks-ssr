/**
 * @file          /ks-ssr-manager/src/daemon/main.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <dbus-c++/glib-integration.h>
#include <glib/gi18n.h>
#include <gtk3-log-i.h>
#include "src/daemon/categories.h"
#include "src/daemon/configuration.h"
#include "src/daemon/dbus.h"
#include "src/daemon/plugins.h"

using namespace KS;

::DBus::Glib::BusDispatcher dispatcher;

struct CommandOptions
{
    CommandOptions() : show_version(false) {}
    bool show_version;
};

int main(int argc, char* argv[])
{
    CommandOptions options;
    Gio::init();

    auto program_name = Glib::path_get_basename(argv[0]);
    klog_gtk3_init(std::string(), "kylinsec-system", PROJECT_NAME, program_name.c_str());

    setlocale(LC_ALL, "");
    bindtextdomain(PROJECT_NAME, SSR_LOCALEDIR);
    bind_textdomain_codeset(PROJECT_NAME, "UTF-8");
    textdomain(PROJECT_NAME);

    Glib::OptionContext context;
    Glib::OptionGroup group(program_name, "group options");

    group.add_entry(MiscUtils::create_option_entry("version", N_("Output version infomation and exit.")),
                    options.show_version);

    group.set_translation_domain(PROJECT_NAME);
    context.set_main_group(group);

    try
    {
        context.parse(argc, argv);
    }
    catch (const Glib::Exception& e)
    {
        KLOG_WARNING("%s", e.what().c_str());
        return EXIT_FAILURE;
    }

    if (options.show_version)
    {
        g_print("%s: %s\n", program_name.c_str(), PROJECT_VERSION);
        return EXIT_SUCCESS;
    }

    Daemon::Configuration::global_init(SSR_INSTALL_DATADIR "/ssr.ini");
    Daemon::Categories::global_init();
    Daemon::Plugins::global_init(Daemon::Configuration::get_instance());

    try
    {
        ::DBus::default_dispatcher = &dispatcher;
        dispatcher.attach(NULL);

        ::DBus::Connection connection = ::DBus::Connection::SystemBus();
        connection.request_name(SSR_DBUS_NAME);
        Daemon::DBus::global_init(connection);
    }
    catch (const DBus::Error& e)
    {
        KLOG_WARNING("%s: %s.", e.name(), e.message());
    }

    auto loop = Glib::MainLoop::create();
    loop->run();

    Daemon::DBus::global_deinit();
    Daemon::Plugins::global_deinit();
    Daemon::Categories::global_deinit();
    Daemon::Configuration::global_deinit();
    return 0;
}