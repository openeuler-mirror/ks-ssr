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

#include "lib/base/file-lock.h"

#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace KS
{
FileLock::FileLock(int32_t file_descriptor, FileLockType lock_type) : file_descriptor_(file_descriptor),
                                                                      lock_type_(lock_type)
{
    int32_t lock_result = 0;
    if (this->file_descriptor_ <= 0)
    {
        this->lock_type_ = FileLockType::FILE_LOCK_TYPE_LAST;
        return;
    }

    switch (lock_type)
    {
    case FileLockType::FILE_LOCK_TYPE_SHARE:
        lock_result = flock(this->file_descriptor_, LOCK_SH);
        break;
    case FileLockType::FILE_LOCK_TYPE_EXCLUSIVE:
        lock_result = flock(this->file_descriptor_, LOCK_EX);
    default:
        break;
    }

    if (lock_result < 0)
    {
        this->lock_type_ = FileLockType::FILE_LOCK_TYPE_LAST;
    }
}

FileLock::~FileLock()
{
    if ((this->lock_type_ == FileLockType::FILE_LOCK_TYPE_EXCLUSIVE ||
         this->lock_type_ == FileLockType::FILE_LOCK_TYPE_SHARE) &&
        this->file_descriptor_ > 0)
    {
        flock(this->file_descriptor_, LOCK_UN);
    }

    if (this->file_descriptor_ > 0)
    {
        close(this->file_descriptor_);
    }
}

QSharedPointer<FileLock> FileLock::createShareLock(const QString &path,
                                                   int flags,
                                                   int mode)
{
    auto file_descriptor = open(path.toLatin1().data(), flags, mode);
    RETURN_VAL_IF_FALSE(file_descriptor > 0, QSharedPointer<FileLock>());
    auto file_lock = QSharedPointer<FileLock>(new FileLock(file_descriptor, FileLockType::FILE_LOCK_TYPE_SHARE));
    RETURN_VAL_IF_FALSE(file_lock->isValid(), QSharedPointer<FileLock>());
    return file_lock;
}

QSharedPointer<FileLock> FileLock::createExcusiveLock(const QString &path,
                                                      int flags,
                                                      int mode)
{
    auto file_descriptor = open(path.toLatin1().data(), flags, mode);
    RETURN_VAL_IF_FALSE(file_descriptor > 0, QSharedPointer<FileLock>());
    auto file_lock = QSharedPointer<FileLock>(new FileLock(file_descriptor, FileLockType::FILE_LOCK_TYPE_EXCLUSIVE));
    RETURN_VAL_IF_FALSE(file_lock->isValid(), QSharedPointer<FileLock>());
    return file_lock;
}
}  // namespace KS
