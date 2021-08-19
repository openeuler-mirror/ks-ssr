/**
 * @file          /kiran-ssr-manager/lib/base/file-lock.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
 */

#pragma once

#include <memory>
#include "lib/base/def.h"

namespace Kiran
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

    int32_t get_file_descriptor() { return this->file_descriptor_; };

    bool is_valid() { return (this->file_descriptor_ > 0 && this->lock_type_ != FileLockType::FILE_LOCK_TYPE_LAST); };

    static std::shared_ptr<FileLock> create_share_lock(const std::string &path,
                                                       int flags,
                                                       int mode);

    static std::shared_ptr<FileLock> create_excusive_lock(const std::string &path,
                                                          int flags,
                                                          int mode);

private:
    int32_t file_descriptor_;
    FileLockType lock_type_;
};
}  // namespace Kiran
