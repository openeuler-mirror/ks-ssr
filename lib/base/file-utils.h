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

#include "lib/base/base.h"

namespace KS
{

class FileUtils
{
public:
    FileUtils(){};
    virtual ~FileUtils(){};

    // 获取文件内容，该函数会添加文件锁
    static bool readContentsWithLock(const QString &path, QString &contents);
    // 写入文件内容，该函数会添加文件锁
    static bool writeContentsWithLock(const QString &path, const QString &contents);
    // Glib::file_set_contents调用了rename函数，这里使用write函数写入内容到文件避免产生文件删除事件
    static bool writeContents(const QString &path, const QString &contents);
};

}  // namespace KS