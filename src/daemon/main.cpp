/**
 * @file          /kiran-sse-manager/src/daemon/main.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <glib/gi18n.h>
#include <gtk3-log-i.h>
#include "src/daemon/sse-categories.h"
#include "src/daemon/sse-configuration.h"
#include "src/daemon/sse-manager.h"
#include "src/daemon/sse-plugins.h"
#include "sse-config.h"

int main(int argc, char* argv[])
{
    Gio::init();

    auto program_name = Glib::path_get_basename(argv[0]);
    klog_gtk3_init(std::string(), "kylinsec-system", PROJECT_NAME, program_name.c_str());

    setlocale(LC_ALL, "");
    bindtextdomain(PROJECT_NAME, SSE_LOCALEDIR);
    bind_textdomain_codeset(PROJECT_NAME, "UTF-8");
    textdomain(PROJECT_NAME);

    Glib::OptionContext context;
    Glib::OptionGroup group(program_name, "group options");

    // version
    Glib::OptionEntry version_entry;
    version_entry.set_long_name("version");
    version_entry.set_flags(Glib::OptionEntry::FLAG_NO_ARG);
    version_entry.set_description(N_("Output version infomation and exit"));

    group.add_entry(version_entry, [program_name](const Glib::ustring& option_name, const Glib::ustring& value, bool has_value) -> bool {
        g_print("%s: %s\n", program_name.c_str(), PROJECT_VERSION);
        return true;
    });

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

    Kiran::SSEConfiguration::global_init(SSE_INSTALL_DATADIR "/sse.ini");
    Kiran::SSECategories::global_init();
    Kiran::SSEPlugins::global_init(Kiran::SSEConfiguration::get_instance());
    Kiran::SSEManager::global_init();

    auto loop = Glib::MainLoop::create();
    loop->run();
    return 0;
}