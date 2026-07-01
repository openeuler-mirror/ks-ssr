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

/*
KV类型配置：
    每行是一个分割符分割的两列组成的键值(key-value)对。
    写入时首先判断文件是否已经存在key，如果存在则直接替换value，否则新增一行。
*/
namespace KS
{
namespace Config
{
class KV
{
public:
    /**
     * @brief 为一个配置文件创建ConfigPlain
     * @param {conf_path}  配置文件路径。
     * @param {kv_split_pattern}  分割键值对的正则表达式
     * @param {kv_join_str}  拼接键值对的字符串
     * @param {comment} 注释字符串
     * @return {}
     */
    KV(const QString &conf_path,
       const QString &kv_split_pattern,
       const QString &kv_join_str,
       const QString &comment);

public:
    virtual ~KV();

    bool get(const QString &key, QString &value);
    // 如果key不存在则自动创建
    bool set(const QString &key, const QString &value);
    // 设置所有key值匹配项
    bool setAll(const QString &key, const QString &value);
    bool del(const QString &key);

private:
    QString conf_path_;
    QString kv_split_pattern_;
    QString kv_join_str_;
    QString comment_;
};
}  // namespace Config
}  // namespace KS
