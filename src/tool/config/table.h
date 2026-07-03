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

#include <QString>
#include <QVector>
#include "lib/base/base.h"

/* TABLE类型配置： 每行是一个分割符分割的多列数据，每行的列数量可以不相同 */

namespace KS
{
namespace Config
{
class Table
{
public:
    /**
     * @brief 为一个配置文件创建ConfigPlain
     * @param {conf_path}  配置文件路径。
     * @param {split_pattern}  分割列的正则表达式，如果为空，表示每一行一个key，没有value
     * @param {join_str}  拼接列的字符串
     * @return {}
     */
    Table(const QString &conf_path,
          const QString &split_pattern,
          const QString &join_str);

public:
    virtual ~Table();

    // 返回第一个匹配pred条件的行
    bool get(std::function<bool(QVector<QString>)> pred, QString &value);
    // 替换第一个满足pred条件的行，如果不存在则插入新行
    bool set(const QString &newline, std::function<bool(QVector<QString>)> pred);
    // 删除所有满足pred的行
    bool del(std::function<bool(QVector<QString>)> pred);

private:
    QString conf_path_;
    QString split_pattern_;
    QString join_str_;
};
}  // namespace Config
}  // namespace KS
