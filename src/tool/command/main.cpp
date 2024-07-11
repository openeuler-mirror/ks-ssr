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
 * Author:     zhenggongping <zhenggongping@kylinos.com.cn>
 */

#include <qt5-log-i.h>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QLocale>
#include <QTextStream>
#include <QTranslator>
#include <iostream>
#include "cmd-parser.h"
#include "include/ssr-i.h"

using namespace KS;

QString leftJustify(const QString &str, int width)
{
    QChar fillChar = ' ';
    int strWidth = 0;
    for (const QChar &ch : str)
    {
        if (ch.unicode() < 128)
        {
            strWidth += 1;
        }
        else
        {
            strWidth += 2;
        }
    }
    if (strWidth >= width)
    {
        return str;
    }
    return str + QString(width - strWidth, fillChar);
}

void helpTextOut(QString helpText, QString firstLine = "", bool removeDoubleLine = false)
{
    QTextStream cerr(stderr);
    auto textList = helpText.split("\n");

    if (!firstLine.isEmpty())
    {
        auto oldFirstLine = textList.first();
        auto firstLineList = oldFirstLine.split(" ").mid(0, 2);
        firstLineList.append(firstLine);

        textList.replace(0, firstLineList.join(" "));
    }

    // 移除　--help
    QStringList newTextList;
    for (auto text : textList)
    {
        if (!text.contains("--help"))
        {
            newTextList.append(text);
        }
    }
    // 移除　空的Options
    // 前后为空
    for (int i = 0; i < newTextList.size(); i++)
    {
        if ("Options:" == newTextList.at(i) &&
            i - 1 >= 0 &&
            i + 1 < newTextList.size() &&
            newTextList.at(i - 1).isEmpty() &&
            newTextList.at(i + 1).isEmpty())
        {
            newTextList.removeAt(i);
            newTextList.removeAt(i);  // 随后的空行也移除
            break;
        }
    }

    if (removeDoubleLine)
    {
        for (auto &text : newTextList)
        {
            text.remove("--");
        }
    }

    cerr << newTextList.join("\n");
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    klog_qt5_init(SSR_ZLOG_CONFIG_FILE, "kylinsec-session", PROJECT_NAME, app.applicationName().toLatin1());

    QTranslator translator;
    if (!translator.load(QLocale(), qAppName(), ".", SSR_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        KLOG_WARNING() << "Load Translator File failed :" << QLocale().name() << SSR_INSTALL_TRANSLATIONDIR << qAppName();
    }
    else
    {
        app.installTranslator(&translator);
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("This tool is mainly used in non-graphical system scenarios as a command line alternative to ks-ssr-gui."));
    parser.addHelpOption();
    QCommandLineOption moduleOption("module", QObject::tr("Specify the operation module, br - baseline hardening, vulnerability - vulnerability fixing."), QString("br|vulnerability"));
    parser.addOption(moduleOption);
    QCommandLineOption scanOption("scan", QObject::tr("One-click scanning"));
    parser.addOption(scanOption);
    QCommandLineOption reinforceOption("reinforce", QObject::tr("name - Specify reinforcement items to be reinforced, multiple reinforcement items are separated by comma; All - One-click reinforcement"), "name", "All");
    parser.addOption(reinforceOption);
    QCommandLineOption repairOption("repair", QObject::tr("name - specify the vulnerability to fix, multiple vulnerabilities are separated by commas; All - One-click repair"), "name", "All");
    parser.addOption(repairOption);
    QCommandLineOption outputOption("output", QObject::tr("Output results to file"));
    parser.addOption(outputOption);
    QCommandLineOption exportOption("export", QObject::tr("Export the report. Input pdf file path"), QString("save_path"));
    parser.addOption(exportOption);
    parser.process(app);
    QString module = parser.value(moduleOption);
    if (module.isEmpty())
    {
        std::cout << QObject::tr("Error: Module not provided.").toStdString() << std::endl;
        parser.showHelp(0);
        return 1;
    }

    bool scanEnabled = parser.isSet(scanOption);
    bool reinforceEnabled = parser.isSet(reinforceOption);
    bool repairEnabled = parser.isSet(repairOption);
    bool outputEnabled = parser.isSet(outputOption);
    bool exportEnabled = parser.isSet(exportOption);
    QString exportPath = parser.value(exportOption);
    exportPath = QDir(QDir::currentPath()).absoluteFilePath(exportPath);
    KS::Command::Command cmd_parser;
    cmd_parser.setFileOutput(outputEnabled);
    if ("br" == module && (scanEnabled || reinforceEnabled || exportEnabled))
    {
        if (scanEnabled)
        {
            cmd_parser.brScan();
        }
        else if (reinforceEnabled)
        {
            QString param = parser.value(reinforceOption);
            QStringList names;
            if ("All" != param)
                names = param.split(',', QString::SkipEmptyParts);
            cmd_parser.reinforce(names);
        }
        else
        {
            cmd_parser.exportReport("br", exportPath);
        }
    }
    else if ("vulnerability" == module && (scanEnabled || repairEnabled || exportEnabled))
    {
        if (scanEnabled)
        {
            cmd_parser.vulnerabilityScan();
        }
        else if (repairEnabled)
        {
            QString param = parser.value(repairOption);
            QStringList cves;
            if ("All" != param)
                cves = param.split(',', QString::SkipEmptyParts);
            cmd_parser.repair(cves);
        }
        else
        {
            cmd_parser.exportReport("vulnerability", exportPath);
        }
    }
    else
    {
        std::cout << QObject::tr("Module parameter provided error.").toStdString() << std::endl;
        parser.showHelp(0);
        return 1;
    }

    return app.exec();
}
