/**
 * @file          /ks-ssr-manager/src/tool/crypto/main.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#include <glib/gi18n.h>
#include <gtk3-log-i.h>
#include "src/tool/crypto/cmd-parser.h"

int main(int argc, char* argv[])
{
    Gio::init();

    auto program_name = Glib::path_get_basename(argv[0]);
    klog_gtk3_init(std::string(), "kylinsec-session", PROJECT_NAME, program_name.c_str());
    Gio::init();

    setlocale(LC_ALL, "");
    bindtextdomain(PROJECT_NAME, SSR_LOCALEDIR);
    bind_textdomain_codeset(PROJECT_NAME, "UTF-8");
    textdomain(PROJECT_NAME);

    KS::Crypto::CmdParser cmd_parser;
    cmd_parser.init();
    return cmd_parser.run(argc, argv);
}