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
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */

#pragma once

#include <QObject>
namespace KS
{
enum TrustedFileType
{
    // 未知文件类型
    TRUSTED_FILE_TYPE_NONE = 0,
    // 可执行文件
    TRUSTED_FILE_TYPE_EXECUTABLE_FILE,
    // 动态库
    TRUSTED_FILE_TYPE_DYNAMIC_LIBRARY,
    // 内核模块
    TRUSTED_FILE_TYPE_KERNEL_MODULE,
    // 可执行脚本
    TRUSTED_FILE_TYPE_EXECUTABLE_SCRIPT
};

enum TrustedFileStatus
{
    // 异常 (未认证/被篡改)
    TRUSTED_FILE_STATUS_ILLEGAL = 0,
    // 正常（已认证）
    TRUSTED_FILE_STATUS_NORMAL,
};

struct TrustedRecord
{
    // 是否被选中
    bool selected;
    // 文件路径
    QString filePath;
    // 文件类型
    QString type;
    // 状态
    QString status;
    QString md5;
    // 是否开启防卸载
    bool guard;
};

class TPUtils : public QObject
{
    Q_OBJECT
public:
    TPUtils(QObject *parent = nullptr);
    virtual ~TPUtils();

    static QString fileTypeEnum2Str(int fileType);
    static QString fileStatusEnum2Str(int fileStatus);
signals:
};

}  // namespace KS
