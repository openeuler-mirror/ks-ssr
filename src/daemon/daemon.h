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

#pragma once

#include <QObject>
#include <QSharedPointer>

namespace KS
{
class LicenseProxy;

class Daemon : public QObject
{
    Q_OBJECT
public:
    Daemon();
    virtual ~Daemon();

    static Daemon *getInstance()
    {
        return m_instance;
    };

    static void globalInit()
    {
        m_instance = new Daemon();
        m_instance->init();
    };

    static void globalDeinit() { delete m_instance; };

private:
    void init();
    void start();

private:
    static Daemon *m_instance;

    QSharedPointer<LicenseProxy> m_licenseProxy;
};
}  // namespace KS
