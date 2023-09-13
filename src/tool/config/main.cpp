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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#include "lib/base/base.h"
// #include <glib/gi18n.h>
#include <unistd.h>
#include "src/tool/config/cmd-parser.h"

#include <QCoreApplication>
#include <QTranslator>
#include <iostream>

using namespace KS;

int main(int argc, char* argv[])
{
    // auto program_name = Glib::path_get_basename(argv[0]);
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);
    // klog_gtk3_init(std::string(), "kylinsec-system", PROJECT_NAME, a.applicationName());
    klog_qt5_init(QString(), "kylinsec-system", PROJECT_NAME, a.applicationName());
    // Gio::init();

    // setlocale(LC_ALL, "");
    // bindtextdomain(PROJECT_NAME, BR_LOCALEDIR);
    // bind_textdomain_codeset(PROJECT_NAME, "UTF-8");
    // textdomain(PROJECT_NAME);

    QTranslator translator;
#pragma message("将 cmake 翻译部分完成后修改翻译文件路径")
    if (!translator.load(QLocale(), qAppName(), ".", " ", ".qm"))
    {
        KLOG_WARNING() << "Load translator failed!";
    }
    else
    {
        a.installTranslator(&translator);
    }

    if (getuid() != 0)
    {
#pragma message("无法输出中文")
        std::cout << "Command 'ks-br-config' can only be run as root!" << std::endl;
        // fmt::print(stderr, _("Command 'ks-br-config' can only be run as root!"));
        return EXIT_FAILURE;
    }

    KS::Config::CmdParser cmd_parser;
    cmd_parser.init();
    return cmd_parser.run(argc, argv, a);
}