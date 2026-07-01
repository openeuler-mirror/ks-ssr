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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */

#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>
#include "include/ssr-i.h"
#include "notify.h"

using namespace KS;

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    QTranslator translator;
    if (!translator.load(QLocale(), qAppName(), ".", SSR_INSTALL_TRANSLATIONDIR, ".qm"))
    {
        fprintf(stderr, "Load translator failed!");
    }
    else
    {
        app.installTranslator(&translator);
    }

    Notify::Notify notify;

    return app.exec();
}
