/**
 * @file          /ks-ssr-manager/src/tool/config/main.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "lib/base/base.h"
#include "src/tool/config/cmd-parser.h"

using namespace KS;

int main(int argc, char* argv[])
{
    auto program_name = Glib::path_get_basename(argv[0]);
    klog_gtk3_init(std::string(), "kylinsec-system", PROJECT_NAME, program_name.c_str());
    Gio::init();

    setlocale(LC_ALL, "");
    bindtextdomain(PROJECT_NAME, SSR_LOCALEDIR);
    bind_textdomain_codeset(PROJECT_NAME, "UTF-8");
    textdomain(PROJECT_NAME);

    if (getuid() != 0)
    {
        fmt::print(stderr, _("Command 'ks-ssr-config' can only be run as root!"));
        return EXIT_FAILURE;
    }

    KS::Config::CmdParser cmd_parser;
    cmd_parser.init();
    return cmd_parser.run(argc, argv);
}