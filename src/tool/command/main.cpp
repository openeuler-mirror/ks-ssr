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

    QTextStream cerr(stderr);

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("This tool is mainly used in non-graphical system scenarios as a command line alternative to ks-ssr-gui."));
    parser.addHelpOption();

    QCommandLineOption brOption("br", QObject::tr("baseline hardening"));
    QCommandLineOption vulnerabilityOption("vulnerability", QObject::tr("vulnerability fixing"));

    parser.addOption(brOption);
    parser.addOption(vulnerabilityOption);

    QStringList args = app.arguments();
    args.removeFirst();  // 程序名称移除
    if (args.isEmpty())
    {
        helpTextOut(parser.helpText(), "", true);
        return 1;
    }

    QCommandLineOption scanOption("scan", QObject::tr("One-click scanning"));
    QString reinforceDes = QObject::tr("name - Specify reinforcement items to be reinforced, multiple reinforcement items are separated by comma; All - One-click reinforcement");
    QCommandLineOption reinforceOption("reinforce", reinforceDes, "name", "All");
    QCommandLineOption repairOption("repair", QObject::tr("name - specify the vulnerability to fix, multiple vulnerabilities are separated by commas; All - One-click repair"), "name", "All");
    QCommandLineOption exportOption("export", QObject::tr("Export the report. Input pdf file path"), QString("save_path"));
    QCommandLineOption outputOption("output", QObject::tr("Output results to file"));

    const QString subCommand = args.first();
    int ret = -1;
    KS::Command::Command cmd_parser;
    if (subCommand == "br")
    {
        // 重新定义一个　QCommandLineParser，原因：addOption历史记录会在helpText()中打印
        QCommandLineParser parser;
        parser.setApplicationDescription(QObject::tr("The current selection is br."));
        parser.addHelpOption();

        // -- 或　-　都可以
        parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

        parser.addOption(scanOption);
        parser.addOption(reinforceOption);
        parser.addOption(exportOption);
        parser.addOption(outputOption);
        parser.parse(app.arguments());
        cmd_parser.setFileOutput(parser.isSet(outputOption));
        if (parser.isSet(scanOption))
        {
            ret = cmd_parser.brScan();
        }
        else if (parser.isSet(reinforceOption))
        {
            QString param = parser.value(reinforceOption);
            QStringList names = "All" == param ? QStringList() : param.split(',', QString::SkipEmptyParts);
            ret = cmd_parser.brReinforce(names);
        }
        else if (parser.isSet(exportOption))
        {
            ret = cmd_parser.brExport(parser.value(exportOption));
        }
        else
        {
            helpTextOut(parser.helpText(), "br [options]");
        }
    }
    else if (subCommand == "vulnerability")
    {
        QCommandLineParser parser;
        parser.setApplicationDescription(QObject::tr("The current selection is vulnerability."));
        parser.addHelpOption();

        // -- 或　-　都可以
        parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);

        parser.addOption(scanOption);
        parser.addOption(repairOption);
        parser.addOption(exportOption);
        parser.addOption(outputOption);
        parser.parse(app.arguments());
        cmd_parser.setFileOutput(parser.isSet(outputOption));
        if (parser.isSet(scanOption))
        {
            ret = cmd_parser.vulnerabilityScan();
        }
        else if (parser.isSet(repairOption))
        {
            QString param = parser.value(repairOption);
            QStringList names = "All" == param ? QStringList() : param.split(',', QString::SkipEmptyParts);
            ret = cmd_parser.vulnerabilityRepair(names);
        }
        else if (parser.isSet(exportOption))
        {
            ret = cmd_parser.vulnerabilityExport(parser.value(exportOption));
        }
        else
        {
            helpTextOut(parser.helpText(), "vulnerability [options]");
        }
    }
    else
    {
        cerr << QObject::tr("Invalid module!") << Qt::endl;
        helpTextOut(parser.helpText(), "", true);
        return 1;
    }

    if (-1 == ret)
    {
        return 1;
    }
    return app.exec();
}
