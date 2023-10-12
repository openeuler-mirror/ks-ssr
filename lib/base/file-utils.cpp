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

#include "lib/base/file-utils.h"
#include <fcntl.h>
#include <qt5-log-i.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <QFile>
#include <QString>
#include "lib/base/file-lock.h"

namespace KS
{
bool FileUtils::readContentsWithLock(const QString &path, QString &contents)
{
    QFile file(path);
    auto file_lock = FileLock::createShareLock(path, O_RDONLY, 0);
    if (!file_lock)
    {
        KLOG_DEBUG("Failed to create share lock for %s.", path.toLatin1().data());
        return false;
    }

    if (!file.open(QIODevice::OpenModeFlag::ReadOnly))
    {
        KLOG_WARNING("Failed to get file contents: %s.", path.toLatin1().data());
        return false;
    }
    contents = file.readAll();
    file.close();
    return true;
}

bool FileUtils::writeContentsWithLock(const QString &path, const QString &contents)
{
    auto file_lock = FileLock::createExcusiveLock(path, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING("Failed to lock file %s.", path.toLatin1().data());
        return false;
    }

    KLOG_DEBUG() << "Write contents: " << contents.toLocal8Bit();
    return FileUtils::writeContents(path, contents);
}

bool FileUtils::writeContents(const QString &path, const QString &contents)
{
    QFile file(path);
    KLOG_DEBUG() << "path: " << path.toLocal8Bit();

    bool retval = true;

    /// @todo 循环出口必须要输出一条告警日志？ 这是否意味着这是个死循环，没有正常退出方式？
    do
    {
        if (!file.open(QIODevice::OpenModeFlag::WriteOnly | QIODevice::OpenModeFlag::Truncate))
        {
            KLOG_WARNING() << "Failed to open file " << path.toLocal8Bit() << " : " << strerror(errno);
            retval = false;
            break;
        }

        if (file.write(contents.toLocal8Bit(), contents.length()) == -1)
        {
            KLOG_WARNING() << "Failed to write file " << path.toLocal8Bit() << " : " << strerror(errno);
            retval = false;
            break;
        }

    } while (0);

    file.close();

    return retval;
}
}  // namespace KS
