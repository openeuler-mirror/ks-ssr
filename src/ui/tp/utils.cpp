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
#include "utils.h"
namespace KS
{
namespace TP
{
Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

Utils::~Utils()
{
}

QString Utils::fileTypeEnum2Str(int fileType)
{
    QString type;
    switch (fileType)
    {
    case TrustedFileType::TRUSTED_FILE_TYPE_NONE:
        type = QString(tr("Unknown file"));
        break;
    case TrustedFileType::TRUSTED_FILE_TYPE_EXECUTABLE_FILE:
        type = QString(tr("Executable file"));
        break;
    case TrustedFileType::TRUSTED_FILE_TYPE_DYNAMIC_LIBRARY:
        type = QString(tr("Dynamic library"));
        break;
    case TrustedFileType::TRUSTED_FILE_TYPE_KERNEL_MODULE:
        type = QString(tr("Kernel file"));
        break;
    case TrustedFileType::TRUSTED_FILE_TYPE_EXECUTABLE_SCRIPT:
        type = QString(tr("Executable script"));
        break;
    default:
        break;
    }
    return type;
}

QString Utils::fileStatusEnum2Str(int fileStatus)
{
    QString status;
    if (fileStatus == TrustedFileStatus::TRUSTED_FILE_STATUS_NORMAL)
    {
        status = QString(tr("Certified"));
    }
    else
    {
        status = QString(tr("Being tampered with"));
    }
    return status;
}
}  // namespace TP
}  // namespace KS
