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
 * Author:     wangyucheng <wangyucheng@kylinos.com.cn>
 */

#include "lib/base/misc-utils.h"
#include <QProcess>
#include "lib/base/base.h"

namespace KS
{
MiscUtils::MiscUtils()
{
}

bool MiscUtils::spawnSync(const QList<QString> &argv,
                           QString *standard_output,
                           QString *standard_error)
{
    // KLOG_DEBUG("Exec command: %s.", StrUtils::join(argv.toVector(), " ").toLatin1().data());
    KLOG_DEBUG("Exec command: %s.", QStringList(argv).join(" ").toLatin1());

    QProcess process;
    auto execute = argv[0];
    auto arg = argv;
    arg.removeFirst();
    process.start(execute, arg);
    // 30秒内子进程未完成的话, 则判断执行失败, 此函数会阻塞等待进程完成.
    if (!process.waitForFinished(30000))
    {
        KLOG_WARNING("Failed to exec command %s, exit status: %d.", QStringList(argv).join(" ").toLatin1(), process.exitCode());
        return false;
    }
    standard_output = new QString(process.readAllStandardOutput());
    standard_error = new QString(process.readAllStandardError());
    return true;
}
}  // namespace KS
