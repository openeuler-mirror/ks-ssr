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
 * Author:     chendingjian <chendingjian@kylinsec.com.cn>
 */

#include <QCoreApplication>
#include <QLocale>
#include "include/ssr-i.h"
#include "lib/base/misc-utils.h"
#include "notify.h"

using namespace KS;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // 加载翻译
    QList<QTranslator*> translatorList;
    QStringList translatorFileNames{qAppName(), "ks-ssr-base"};
    for (auto fileName : translatorFileNames)
    {
        auto translator = MiscUtils::installTranslator(fileName);
        if (translator)
        {
            translatorList.append(translator);
        }
    }

    Notify::Notify notify;

    bool ret = app.exec();

    // 卸载翻译
    for (auto translator : translatorList)
    {
        MiscUtils::removeTranslator(translator);
    }

    return ret;
}
