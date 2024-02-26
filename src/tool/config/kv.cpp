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

#include "src/tool/config/kv.h"
#include <fcntl.h>
// #include <set>
// #include <glibmm/regex.h>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>
#include "lib/base/file-lock.h"
#include "lib/base/file-utils.h"

namespace KS
{
namespace Config
{
#define DEFAULT_SPLIT_REGEX "\\s+"
#define DEFAULT_JOIN_STRING "\t"
#define DEFAULT_COMMENT_STRING "#"

KV::KV(const QString &conf_path,
       const QString &kv_split_pattern,
       const QString &kv_join_str,
       const QString &comment)
    : conf_path_(conf_path),
      kv_split_pattern_(kv_split_pattern),
      kv_join_str_(kv_join_str),
      comment_(comment)
{
    if (this->kv_split_pattern_.isEmpty())
    {
        this->kv_split_pattern_ = DEFAULT_SPLIT_REGEX;
    }

    if (this->kv_join_str_.isEmpty())
    {
        this->kv_join_str_ = DEFAULT_JOIN_STRING;
    }

    if (this->comment_.isEmpty())
    {
        this->comment_ = DEFAULT_COMMENT_STRING;
    }
}

KV::~KV()
{
}

bool KV::get(const QString &key, QString &value)
{
    // 文件如果不存在则返回空字符串
    // if (!Glib::file_test(this->conf_path_, Glib::FILE_TEST_EXISTS))
    if (!QFileInfo(this->conf_path_).isFile())
    {
        value = QString();
        return true;
    }

    QString contents;
    RETURN_VAL_IF_FALSE(FileUtils::readContentsWithLock(this->conf_path_, contents), false);

    auto lines = StrUtils::splitLines(contents);
    QRegExp split_field_regex(this->kv_split_pattern_);
    // auto split_field_regex = Glib::Regex::create(this->kv_split_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    for (auto iter = lines.begin(); iter != lines.end(); ++iter)
    {
        auto trim_line = StrUtils::trim(*iter);
        // 忽略空行和注释行
        CONTINUE_IF_TRUE(trim_line.isEmpty() || trim_line[0] == '#');
        QVector<QString> fields = trim_line.split(split_field_regex).toVector();
        // 只考虑两列的行
        CONTINUE_IF_TRUE(fields.size() != 2);
        KLOG_DEBUG() << "Read Line: key: " << fields[0].toLatin1() << " value: " << fields[1].toLatin1();

        if (fields[0] == key)
        {
            value = fields[1];
            return true;
        }
    }

    // 未找到不返回错误
    return true;
}

bool KV::set(const QString &key, const QString &value)
{
    QString new_contents;

    // 在读写期间都不应该让其他进程改动该文件，否则可能会导致结果不一致。
    auto file_lock = FileLock::createExcusiveLock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING() << "Failed to lock file " << this->conf_path_.toLatin1();
        return false;
    }

    QFile file(this->conf_path_);
    file.open(QIODevice::OpenModeFlag::ReadOnly);
    auto contents = file.readAll();

    // auto contents = Glib::file_get_contents(this->conf_path_);
    auto lines = StrUtils::splitLines(contents);

    QRegExp split_field_regex(this->kv_split_pattern_);
    QRegExp second_field_regex(QString("(\\s*\\S+%1)(\\S+)").arg(this->kv_split_pattern_));

    // auto split_field_regex = Glib::Regex::create(this->kv_split_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    // auto second_field_regex = Glib::Regex::create(fmt::format("(\\s*\\S+{0})(\\S+)", this->kv_split_pattern_),
    //                                               Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    bool is_matched = false;
    int32_t match_pos = 0;
    bool is_match_comment = false;
    QString match_line;

    // 寻找匹配key的行，如果没有匹配的非注释行可用，则使用匹配的注释行（注释将被去掉）

    for (const auto &line : lines)
    {
        QVector<QString> fields;

        // 注释行判断需要包括前面的空白字符
        bool is_comment = StrUtils::startswith(line, this->comment_);

        if (is_comment)
        {
            auto trim_line = StrUtils::trim(line.mid(this->comment_.size()));
            fields = trim_line.split(split_field_regex).toVector();
        }
        else
        {
            auto trim_line = StrUtils::trim(line);
            fields = trim_line.split(split_field_regex).toVector();
        }

        if (!is_matched &&
            fields.size() == 2 &&
            fields[0] == key &&
            (match_line.size() == 0 || (int32_t(is_comment) <= int32_t(is_match_comment))))
        {
            match_pos = new_contents.size();
            match_line = line;
            is_match_comment = is_comment;
            is_matched = true;
        }

        new_contents.append(line);
        new_contents.push_back('\n');
        file.close();
    }

    KLOG_DEBUG() << "match line: " << match_line.toLatin1() << " is comment: " << is_match_comment;

    if (match_line.size() > 0)
    {
        if (value.isEmpty())
        {
            // 如果不是注释行，则进行注释，否则不处理
            if (!is_match_comment)
            {
                new_contents.replace(match_pos, match_line.size(), this->comment_ + match_line);
                KLOG_DEBUG() << "Comment line: " << match_line.toLatin1() << " with " << this->comment_.toLatin1();
            }
        }
        else
        {
            // 如果存在注释则去掉，然后对value进行修改
            auto uncomment_line = is_match_comment ? match_line.mid(this->comment_.size()) : match_line;
            auto replace_line = uncomment_line.replace(second_field_regex, "\\1" + value);
            // auto replace_line = second_field_regex.replace(uncomment_line, 0, "\\g<1>" + value, static_cast<Glib::RegexMatchFlags>(0));
            new_contents.replace(match_pos, match_line.size(), replace_line);
            KLOG_DEBUG() << "Replace line: " << match_line.toLatin1() << " with " << replace_line.toLatin1();
        }
    }
    else
    {
        // 如果不存在匹配行且value不为空，则在最后添加一行
        if (!value.isEmpty())
        {
            auto new_line = key + this->kv_join_str_ + value + "\n";
            KLOG_DEBUG() << "New line: " << new_line.toLatin1();
            new_contents.append(new_line);
        }
    }
    KLOG_DEBUG() << "New contents: " << new_contents.toLatin1();
    return FileUtils::writeContents(this->conf_path_, new_contents);
}

bool KV::setAll(const QString &key, const QString &value)
{
    QString new_contents;

    // 在读写期间都不应该让其他进程改动该文件，否则可能会导致结果不一致。
    auto file_lock = FileLock::createExcusiveLock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING() << "Failed to lock file " << this->conf_path_.toLatin1();
        return false;
    }

    QFile file(this->conf_path_);
    file.open(QIODevice::OpenModeFlag::ReadOnly);

    auto contents = file.readAll();
    // auto contents = Glib::file_get_contents(this->conf_path_);
    auto lines = StrUtils::splitLines(contents);
    for (auto line : lines)
    {
        auto for_test = line.toStdString();
    }

    QRegExp split_field_regex(this->kv_split_pattern_);
    QRegExp second_field_regex(QString("(\\s*\\S+%1)(\\S+)").arg(this->kv_split_pattern_));
    // auto split_field_regex = Glib::Regex::create(this->kv_split_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    // auto second_field_regex = Glib::Regex::create(fmt::format("(\\s*\\S+{0})(\\S+)", this->kv_split_pattern_),
    //                                               Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    int32_t match_pos = 0;
    bool is_match_comment = false;
    bool is_matching = false;

    // 寻找匹配key的行，如果没有匹配的非注释行可用，则使用匹配的注释行（注释将被去掉）
    for (const auto &line : lines)
    {
        QString match_line = "";
        QVector<QString> fields;

        // 注释行判断需要包括前面的空白字符
        bool is_comment = StrUtils::startswith(line, this->comment_);

        if (is_comment)
        {
            auto trim_line = StrUtils::trim(line.mid(this->comment_.size()));
            fields = trim_line.split(split_field_regex).toVector();
        }
        else
        {
            auto trim_line = StrUtils::trim(line);
            fields = trim_line.split(split_field_regex).toVector();
        }

        if (fields.size() == 2 &&
            fields[0] == key &&
            (int32_t(is_comment) <= int32_t(is_match_comment)))
        {
            match_pos = new_contents.size();
            match_line = line;
            is_match_comment = is_comment;
        }

        new_contents.append(line);
        new_contents.push_back('\n');

        if (match_line.size() > 0)
        {
            // 匹配成功标志位，如果匹配到，就不添加新的参数
            is_matching = true;
            if (value.isEmpty())
            {
                // 如果不是注释行，则进行注释，否则不处理
                if (!is_match_comment)
                {
                    new_contents.replace(match_pos, match_line.size(), this->comment_ + match_line);
                    KLOG_DEBUG() << "Comment line: " << match_line.toLatin1() << " with " << this->comment_.toLatin1();
                }
            }
            else
            {
                // 如果存在注释则去掉，然后对value进行修改
                auto uncomment_line = is_match_comment ? match_line.mid(this->comment_.size()) : match_line;
                auto replace_line = uncomment_line.replace(second_field_regex, "\\1" + value);
                // auto replace_line = second_field_regex->replace(uncomment_line, 0, "\\g<1>" + value, static_cast<Glib::RegexMatchFlags>(0));
                new_contents.replace(match_pos, match_line.size(), replace_line);
                KLOG_DEBUG() << "contents: " << new_contents.toLatin1() << ", Replace line: " << match_line.toLatin1() << ", with " << replace_line.toLatin1();
            }
        }
    }

    if (!is_matching)
    {
        // 如果不存在匹配行且value不为空，则在最后添加一行
        if (!value.isEmpty())
        {
            auto new_line = key + this->kv_join_str_ + value + "\n";
            KLOG_DEBUG() << "New line: " << new_line.toLatin1();
            new_contents.append(new_line);
        }
    }
    file.close();
    KLOG_DEBUG() << "New contents: " << new_contents.toLatin1();
    return FileUtils::writeContents(this->conf_path_, new_contents);
}

bool KV::del(const QString &key)
{
    // 这里偷了一个懒，合理做法是需要单独实现删除逻辑，set函数不应该执行删除操作
    KLOG_DEBUG() << "Key: " << key.toLatin1();
    return this->set(key, QString());
}

}  // namespace Config
}  // namespace KS
