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
#include "lib/base/base.h"

namespace KS
{
namespace Config
{
class PAM
{
public:
    struct MatchLineInfo
    {
        QString content;
        QString match_line;
        int32_t match_pos;
        bool is_match_comment;
        MatchLineInfo() : match_pos(0), is_match_comment(false) {}
    };

    /**
     * @brief 创建PAM对象
     * @param {conf_path} PAM配置路径。
     * @param {line_match_pattern}  需要操作的行，如果有多个行都匹配，则随机选择一个
     * @param {fallback_line} 如果未找到匹配行，则使用传入的缺省行
     * @return {}
     */
    PAM(const QString &conf_path,
        const QString &line_match_pattern);

    /**
     * @brief 获取匹配行中的键值对
     * @param {key}  指定键
     * @param {kv_split_pattern}  指定分割键值对的正则匹配，如果设置为空，则代表查询key是否存在
     * @param {value}  如果kv_split_pattern设置为空，则value表示key是否存在（被设置为"true"或者"false")，否则value为获取到的值。
     * @return {} 如果出错则返回false,否则返回true
     */
    bool getValue(const QString &key, const QString &kv_split_pattern, QString &value);

    bool setValue(const QString &key,
                  const QString &kv_split_pattern,
                  const QString &value,
                  const QString &kv_join_str);

    bool delValue(const QString &key, const QString &kv_split_pattern);

    // 如果没有找到匹配行，则将fallback_line添加到末尾
    bool addLine(const QString &fallback_line, const QString &next_line_match_regex);
    // 删除匹配行
    bool delLine();
    // 获取匹配行
    bool getLine(QString &line);

private:
    PAM::MatchLineInfo getMatchLine();
    PAM::MatchLineInfo addBehind(const QString &fallback_line, const QString &next_line_match_regex);

    // 将内容写入文件
    bool writeToFile(const QString &content);

    // 最后一个字符是否为空白字符
    bool isWhitespaceInTail(const QString &str);

private:
    QString conf_path_;
    QString line_match_pattern_;
};
}  // namespace Config

}  // namespace KS
