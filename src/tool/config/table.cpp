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

#include "src/tool/config/table.h"
#include <fcntl.h>
#include <QFileInfo>
#include <QRegExp>
#include <set>
#include "lib/base/file-lock.h"
#include "lib/base/file-utils.h"

namespace KS
{
namespace Config
{
#define DEFAULT_JOIN_STRING "\t"

Table::Table(const QString &conf_path,
             const QString &split_pattern,
             const QString &join_str) : conf_path_(conf_path),
                                        split_pattern_(split_pattern),
                                        join_str_(join_str)
{
    if (this->join_str_.isEmpty())
    {
        this->join_str_ = DEFAULT_JOIN_STRING;
    }
}

Table::~Table()
{
}

bool Table::get(std::function<bool(QVector<QString>)> pred, QString &value)
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
    QRegExp split_field_regex(this->split_pattern_);
    // auto split_field_regex = Glib::Regex::create(this->split_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    for (const auto &line : lines)
    {
        QVector<QString> fields;
        auto trim_line = StrUtils::trim(line);
        // 忽略空行和注释行
        CONTINUE_IF_TRUE(trim_line.isEmpty() || trim_line[0] == '#');
        // 如果分割的正则未设置，默认不分割，一整行表示一列
        if (this->split_pattern_.isEmpty())
        {
            fields.push_back(trim_line);
        }
        else
        {
            fields = trim_line.split(split_field_regex).toVector();
        }

        if (pred(fields))
        {
            value = line;
            return true;
        }
    }
    return true;
}

bool Table::set(const QString &newline, std::function<bool(QVector<QString>)> pred)
{
    QString new_contents;

    // 在读写期间都不应该让其他进程改动该文件，否则可能会导致结果不一致。
    auto file_lock = FileLock::createExcusiveLock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING("Failed to lock file %s.", this->conf_path_.toLatin1());
        return false;
    }

    QFile file(this->conf_path_);
    file.open(QIODevice::OpenModeFlag::ReadOnly);
    auto contents = file.readAll();
    // auto contents = Glib::file_get_contents(this->conf_path_);
    auto lines = StrUtils::splitLines(contents);

    QRegExp split_field_regex(this->split_pattern_);
    QRegExp second_field_regex(QString("(\\s*\\S+%1)(\\S+)").arg(this->split_pattern_));
    // auto split_field_regex = Glib::Regex::create(this->split_pattern_, Glib::RegexCompileFlags::REGEX_OPTIMIZE);
    // auto second_field_regex = Glib::Regex::create(fmt::format("(\\s*\\S+{0})(\\S+)", this->split_pattern_),
    //                                               Glib::RegexCompileFlags::REGEX_OPTIMIZE);

    bool has_matched = false;

    for (const auto &line : lines)
    {
        auto trim_line = StrUtils::trim(line);
        QVector<QString> fields;

        // 如果分割的正则未设置，默认不分割，一整行表示一列
        if (this->split_pattern_.isEmpty())
        {
            fields.push_back(trim_line);
        }
        else
        {
            fields = trim_line.split(split_field_regex).toVector();
        }

        if (trim_line.isEmpty() || trim_line[0] == '#' || !pred(fields) || has_matched)
        {
            new_contents.append(line);
            new_contents.push_back('\n');
            continue;
        }

        if (!newline.isEmpty())
        {
            KLOG_DEBUG("Replace line: %s with %s.", line.toLatin1(), newline.toLatin1());
            new_contents.append(newline);
            new_contents.push_back('\n');
            has_matched = true;
        }
        else
        {
            // 值为空则直接删除该行
            KLOG_DEBUG("Delete line: %s.", line.toLatin1());
        }
    }

    // 如果不存在该行且设置的值不为空，则在最后添加一行
    if (!has_matched && !newline.isEmpty())
    {
        KLOG_DEBUG("New line: %s.", newline.toLatin1());
        new_contents.append(newline);
        new_contents.push_back('\n');
    }

    KLOG_DEBUG("New contents: %s.", new_contents.toLatin1());
    return FileUtils::writeContents(this->conf_path_, new_contents);
}

bool Table::del(std::function<bool(QVector<QString>)> pred)
{
    return this->set(QString(), pred);
}

}  // namespace Config
}  // namespace KS
