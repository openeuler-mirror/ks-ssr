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
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);
    klog_qt5_init(QString(), "kylinsec-system", PROJECT_NAME, a.applicationName());
    QTranslator translator;
    if (!translator.load(QLocale(), "ks-ssr-daemon", ".", SSR_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING() << "Load translator failed!";
    }
    else
    {
        a.installTranslator(&translator);
    }

    if (getuid() != 0)
    {
        std::cout << QObject::tr("Command 'ks-br-config' can only be run as root!").toStdString() << std::endl;
        return EXIT_FAILURE;
    }

    KS::Config::CmdParser cmd_parser;
    cmd_parser.init();
    return cmd_parser.run(argc, argv, a);
}