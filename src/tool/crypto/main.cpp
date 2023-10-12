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

// #include <glib/gi18n.h>
// #include <gtk3-log-i.h>
#include <qt5-log-i.h>
#include <QTranslator>
#include "src/tool/crypto/cmd-parser.h"

int main(int argc, char* argv[])
{
    // Gio::init();
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);
    QCommandLineParser parser;

    klog_qt5_init(QString(), "kylinsec-session", PROJECT_NAME, a.applicationName().toLatin1());

    QTranslator translator;
#pragma message("将 cmake 翻译部分完成后修改翻译文件路径")
    if (!translator.load(QLocale(), "ks-ssr-daemon", ".", SSR_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING() << "Load translator failed!";
    }
    else
    {
        a.installTranslator(&translator);
    }

    KS::Crypto::CmdParser cmd_parser;
    cmd_parser.init();
    return cmd_parser.run(a);
}