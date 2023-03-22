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

#ifndef DAEMONMANAGER_H
#define DAEMONMANAGER_H

#include <QObject>
#include "src/daemon/box/box-manager.h"

namespace KS
{
class DaemonManager : public QObject
{
    Q_OBJECT
public:
    DaemonManager();
    virtual ~DaemonManager();

    static DaemonManager *getInstance()
    {
        return m_instance;
    };

    static void globalInit()
    {
        m_instance = new DaemonManager();
        m_instance->init();
    };

    static void globalDeinit() { delete m_instance; };

    BoxManager *boxManger;

private:
    void init();

private:
    static DaemonManager *m_instance;
};
}  // namespace KS

#endif  // DAEMONMANAGER_H
