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
namespace Log
{
struct LogInfo;

class Utils : public QObject
{
    Q_OBJECT
public:
    // 日志类型转换
    static QString logTypeEnum2Str(uint type);
    static int str2LogTypeEnum(const QString &typeStr);
    // 用户名转换
    static QString accountRoleEnum2Str(uint role);
    static int str2AccountRoleEnum(const QString &roleStr);
    // 反序列号化
    static void deserialize(const QStringList &logs, QList<LogInfo> &logInfos);

private:
    // 后台返回的枚举值都为字符串，需要进行转化成enum
    static int logStrType2Enum(const QString &logStr);
    static int roleStrType2Enum(const QString &roleStr);
};
}  // namespace Log
}  // namespace KS
