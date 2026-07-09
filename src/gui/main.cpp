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
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#include <qt5-log-i.h>
#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QFontDatabase>
#include <QScreen>
#include <QTranslator>
#include <QtGlobal>
#include "config-ui.h"
#include "lib/widgets/single-application/single-application.h"
#include "window.h"

using namespace KS;

int main(int argc, char *argv[])
{
    auto argv0 = QFileInfo(argv[0]);
    auto programName = argv0.baseName();

    if (klog_qt5_init(SSR_ZLOG_CONFIG_FILE, "kylinsec-session", PROJECT_NAME, programName) < 0)
    {
        fprintf(stderr, "Failed to init kiran-log.");
    }

    SingleApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    SingleApplication app(argc,
                          argv,
                          true,
                          SingleApplication::Mode::User | SingleApplication::Mode::SecondaryNotification);

    SingleApplication::setApplicationName(programName);
    SingleApplication::setApplicationVersion(PROJECT_VERSION);
    app.setStyle("Fusion");
    QTranslator translator;

    if (!translator.load(QLocale(), qAppName(), ".", SSR_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING() << "Load translator failed!";
    }
    else
    {
        app.installTranslator(&translator);
    }

    if (!app.isPrimary())
    {
        exit(EXIT_SUCCESS);
    }

    KS::Window window;

    QRect rect = app.primaryScreen()->geometry();
    auto screens = app.screens();
    if (screens.count() > 1)
    {
        QPoint pos = QCursor::pos();
        for (auto &screen : screens)
        {
            if (screen->geometry().contains(pos))
            {
                rect = screen->geometry();
                break;
            }
        }
    }
    window.move((rect.width() - window.width()) / 2, ((rect.height() - window.height()) / 2));

    window.start();

    return app.exec();
}
