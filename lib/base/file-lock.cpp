/**
 * @file          /ks-ssr-manager/lib/base/file-lock.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020 KylinSec. All rights reserved. 
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

std::shared_ptr<FileLock> FileLock::create_share_lock(const std::string &path,
                                                      int flags,
                                                      int mode)
{
    auto file_descriptor = open(path.c_str(), flags, mode);
    RETURN_VAL_IF_FALSE(file_descriptor > 0, nullptr);
    auto file_lock = std::make_shared<FileLock>(file_descriptor, FileLockType::FILE_LOCK_TYPE_SHARE);
    RETURN_VAL_IF_FALSE(file_lock->is_valid(), nullptr);
    return file_lock;
}

std::shared_ptr<FileLock> FileLock::create_excusive_lock(const std::string &path,
                                                         int flags,
                                                         int mode)
{
    auto file_descriptor = open(path.c_str(), flags, mode);
    RETURN_VAL_IF_FALSE(file_descriptor > 0, nullptr);
    auto file_lock = std::make_shared<FileLock>(file_descriptor, FileLockType::FILE_LOCK_TYPE_EXCLUSIVE);
    RETURN_VAL_IF_FALSE(file_lock->is_valid(), nullptr);
    return file_lock;
}
}  // namespace KS
