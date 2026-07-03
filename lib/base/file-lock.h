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

#pragma once

// #include <memory>
#include <QSharedPointer>
#include <QString>
#include "include/ssr-marcos.h"

namespace KS
{
enum FileLockType
{
    // 未加锁
    FILE_LOCK_TYPE_NONE = 0,
    // 共享锁/读锁
    FILE_LOCK_TYPE_SHARE,
    // 互斥锁/写锁
    FILE_LOCK_TYPE_EXCLUSIVE,
    FILE_LOCK_TYPE_LAST
};

class FileLock
{
public:
    FileLock(int32_t file_descriptor, FileLockType lock_type);
    virtual ~FileLock();

    int32_t getFileDescriptor() { return this->file_descriptor_; };

    bool isValid() { return (this->file_descriptor_ > 0 && this->lock_type_ != FileLockType::FILE_LOCK_TYPE_LAST); };

    static QSharedPointer<FileLock> createShareLock(const QString &path,
                                                      int flags,
                                                      int mode);

    static QSharedPointer<FileLock> createExcusiveLock(const QString &path,
                                                         int flags,
                                                         int mode);

private:
    int32_t file_descriptor_;
    FileLockType lock_type_;
};
}  // namespace KS
