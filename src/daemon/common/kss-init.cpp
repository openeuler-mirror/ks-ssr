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
#include "kss-init.h"
#include <qt5-log-i.h>
#include <QProcess>

namespace KS
{
#define KSS_INIT_DATA_CMD "kss secure setup"

KssInit::KssInit(QObject *parent) : QThread(parent)
{
}

void KssInit::run()
{
    auto process = new QProcess(this);
    auto cmd = QString("%1 %2").arg(KSS_INIT_DATA_CMD, "/boot/vmlinuz-`uname -r`");
    KLOG_DEBUG() << "Start executing the command. cmd = " << cmd;
    process->start("bash", QStringList() << "-c" << cmd);
    process->waitForFinished(-1);
}

}  // namespace KS
