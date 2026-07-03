/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
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

#ifndef DAEMON_H
#define DAEMON_H

#include <QObject>

namespace KS
{
class KSSDbus;

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

private:
    static Daemon *m_instance;

    KSSDbus *m_kssDBus;
};
}  // namespace KS

#endif  // DAEMON_H
