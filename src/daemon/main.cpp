/**
 * @file          /ks-sc/src/daemon/main.cpp
 * @brief         
 * @author        chendingjian <chendingjian@kylinos.com>
 * @copyright (c) 2023 KylinSec. All rights reserved.
 */

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QTranslator>
#include <QtDBus/QDBusConnection>
#include <QtGlobal>
#include <iostream>
#include "config.h"
#include "src/daemon/daemon-manager.h"

using namespace KS;

int main(int argc, char *argv[])
{
    auto argv0 = QFileInfo(argv[0]);
    auto programName = argv0.baseName();

    if (klog_qt5_init(QString(), "kylinsec-system", PROJECT_NAME, programName) < 0)
    {
        fprintf(stderr, "Failed to init kiran-log.");
    }

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(programName);
    QCoreApplication::setApplicationVersion(PROJECT_VERSION);

    QTranslator translator;

    if (!translator.load(QLocale(), qAppName(), ".", SC_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING() << "Load translator failed!";
    }
    else
    {
        app.installTranslator(&translator);
    }

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    // parser.addOption();

    parser.process(app);

    KS::DaemonManager::globalInit();

    auto retval = app.exec();

    return retval;
}
