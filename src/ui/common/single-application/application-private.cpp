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
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */
#include "application-private.h"
// #include "style.h"

#include <QDebug>
#include <QLibraryInfo>
#include <QTranslator>
namespace KS
{
ApplicationPrivate::ApplicationPrivate(Application* ptr)
{
}

ApplicationPrivate::~ApplicationPrivate()
{
}

void ApplicationPrivate::init()
{
    setupTranslations();
}

void ApplicationPrivate::setupTranslations()
{
    QTranslator* translator = new QTranslator(this);
    if (translator->load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
    {
        QCoreApplication::installTranslator(translator);
    }
    else
    {
        delete translator;
    }
}
}  // namespace KS
