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

#include "src/tool/config/pam.h"
#include <fcntl.h>
#include <QFile>
#include <QRegExp>
#include "lib/base/file-lock.h"
#include "lib/base/file-utils.h"

namespace KS
{
namespace Config
{
PAM::PAM(const QString &conf_path,
         const QString &line_match_regex)
    : conf_path_(conf_path),
      line_match_pattern_(line_match_regex)
{
}

bool PAM::getValue(const QString &key, const QString &kv_split_pattern, QString &value)
{
    QString contents;
    RETURN_VAL_IF_FALSE(FileUtils::readContentsWithLock(this->conf_path_, contents), false);
    auto lines = StrUtils::splitLines(contents);
    QRegExp line_match_regex(this->line_match_pattern_);

    QString kv_pattern;
    if (kv_split_pattern.isEmpty())
    {
        kv_pattern = key;
        value = "false";
    }
    else
    {
        kv_pattern = QString("(%1[\\s]*%2[\\s]*)(\\S+)").arg(key, kv_split_pattern);
    }
    QRegExp kv_regex(kv_pattern);
    QRegExp split_field_regex(kv_split_pattern);

    for (auto iter = lines.begin(); iter != lines.end(); ++iter)
    {
        // Glib::MatchInfo match_info;
        auto trim_line = StrUtils::trim(*iter);
        // 忽略空行和注释行
        CONTINUE_IF_TRUE(trim_line.isEmpty() || trim_line[0] == '#');
        CONTINUE_IF_TRUE(!(line_match_regex.indexIn(*iter) != -1));
        CONTINUE_IF_TRUE(kv_regex.indexIn(*iter) < 0);
        if (!kv_split_pattern.isEmpty())
        {
            kv_regex.indexIn(*iter);
            auto fields = kv_regex.cap(0).split(split_field_regex).toVector();
            value = fields[1].toLatin1();
            KLOG_DEBUG("Read Line: key: %s, value: %s.", fields[0].toLocal8Bit().toStdString().c_str(), fields[1].toLocal8Bit().toStdString().c_str());
        }
        else
        {
            value = "true";
        }
        return true;
    }
    return true;
}

bool PAM::setValue(const QString &key,
                   const QString &kv_split_pattern,
                   const QString &value,
                   const QString &kv_join_str)
{
    // 在读写期间都不应该让其他进程改动该文件，否则可能会导致结果不一致。
    auto file_lock = FileLock::createExcusiveLock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING() << "Failed to lock file " << this->conf_path_.toLatin1();
        return false;
    }
    auto match_info = this->getMatchLine();

    if (match_info.match_line.size() > 0 && !match_info.is_match_comment)
    {
        QString kv_pattern(kv_split_pattern.isEmpty() ? QString("(%1)").arg(key) : QString("(%1[\\s]*%2[\\s]*)(\\S+)").arg(key, kv_split_pattern));
        // auto kv_pattern = kv_split_pattern.isEmpty() ? fmt::format("({0})", key) : fmt::format("({0}[\\s]*{1}[\\s]*)(\\S+)", key, kv_split_pattern);
        QRegExp kv_regex(kv_pattern);
        // auto kv_regex = Glib::Regex::create(kv_pattern);
        QString replace_line = match_info.match_line;
        if (kv_regex.indexIn(match_info.match_line) != -1)
        {
            // 修改键值对
            if (!kv_split_pattern.isEmpty() && !value.isEmpty())
            {
                replace_line.replace(kv_regex, "\\1" + value);
            }
        }
        else
        {
            // 添加键值对
            replace_line += (this->isWhitespaceInTail(match_info.match_line) ? "" : " ") + key;
            if (kv_split_pattern.isEmpty() || value.isEmpty())
            {
                KLOG_WARNING("Unknown situation.");
            }
            replace_line += kv_join_str + value;
        }

        match_info.content.replace(match_info.match_pos, match_info.match_line.size(), replace_line);
        KLOG_DEBUG("Replace line: %s with %s.", match_info.match_line.toLocal8Bit().data(), replace_line.toLocal8Bit().data());
        return this->writeToFile(match_info.content);
    }
    return true;
}

bool PAM::delValue(const QString &key, const QString &kv_split_pattern)
{
    // 在读写期间都不应该让其他进程改动该文件，否则可能会导致结果不一致。
    auto file_lock = FileLock::createExcusiveLock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING() << "Failed to lock file " << this->conf_path_.toLatin1();
        return false;
    }
    auto match_info = this->getMatchLine();
    QRegExp kv_pattern(kv_split_pattern.isEmpty() ? QString("(%1)").arg(key) : QString("(%1[\\s]*%2[\\s]*)(\\S+)").arg(key, kv_split_pattern));
    QRegExp kv_regex(kv_pattern);
    if (match_info.match_line.size() > 0 &&
        !match_info.is_match_comment &&
        kv_regex.indexIn(match_info.match_line) != -1)
    {
        auto replace_line = match_info.match_line.replace(kv_regex, "");
        match_info.content.replace(match_info.match_pos, match_info.match_line.size(), replace_line);
        KLOG_DEBUG() << "Replace line: " << match_info.match_line.toLatin1() << ", with " << replace_line.toLatin1();
        return this->writeToFile(match_info.content);
    }
    return true;
}

bool PAM::addLine(const QString &fallback_line, const QString &next_line_match_regex)
{
    // 在读写期间都不应该让其他进程改动该文件，否则可能会导致结果不一致。
    auto file_lock = FileLock::createExcusiveLock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING() << "Failed to lock file " << this->conf_path_.toLatin1();
        return false;
    }

    auto match_info = this->getMatchLine();

    // 如果匹配到注释行，则取消注释
    if (match_info.match_line.size() > 0 && match_info.is_match_comment)
    {
        auto replace_line = match_info.match_line.mid(1);
        match_info.content.replace(match_info.match_pos, match_info.match_line.size(), replace_line);
        KLOG_DEBUG() << "Replace line: " << match_info.match_line.toLatin1() << ", with " << replace_line.toLatin1();
        this->writeToFile(match_info.content);
    }
    // 如果未匹配到行，则添加新行
    else if (match_info.match_line.size() == 0 && fallback_line.size() > 0)
    {
        KLOG_DEBUG() << "New line: " << fallback_line;
        auto new_match_info = this->addBehind(fallback_line, next_line_match_regex);
        this->writeToFile(new_match_info.content);
    }
    return true;
}

bool PAM::delLine()
{
    // 在读写期间都不应该让其他进程改动该文件，否则可能会导致结果不一致。
    auto file_lock = FileLock::createExcusiveLock(this->conf_path_, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING() << "Failed to lock file " << this->conf_path_.toLatin1();
        return false;
    }

    auto match_info = this->getMatchLine();

    if (match_info.match_line.size() > 0 && !match_info.is_match_comment)
    {
        match_info.content.replace(match_info.match_pos, match_info.match_line.size(), "#" + match_info.match_line);
        KLOG_DEBUG() << "Comment line: " << match_info.match_line.toLatin1() << " with #.";
        this->writeToFile(match_info.content);
    }
    return true;
}

bool PAM::getLine(QString &line)
{
    auto file_lock = FileLock::createShareLock(this->conf_path_, O_RDONLY, 0);
    if (!file_lock)
    {
        KLOG_DEBUG() << "Failed to create share lock for " << this->conf_path_.toLatin1();
        return false;
    }

    auto match_info = this->getMatchLine();

    if (match_info.match_line.size() > 0 && !match_info.is_match_comment)
    {
        line = match_info.match_line;
    }
    return true;
}

PAM::MatchLineInfo PAM::getMatchLine()
{
    MatchLineInfo retval;

    QFile file(this->conf_path_);

    if (!file.open(QIODevice::OpenModeFlag::ReadOnly))
    {
        KLOG_WARNING() << "open file fail, error is " << file.errorString() << "path is " << conf_path_;
        return retval;
    }
    auto contents = file.readAll();
    auto lines = StrUtils::splitLines(contents);
    QRegExp line_match_regex(this->line_match_pattern_);

    // 寻找匹配行，如果没有匹配的非注释行可用，则使用匹配的注释行（注释将被去掉）
    for (const auto &line : lines)
    {
        QVector<QString> fields;

        // 注释行判断需要包括前面的空白字符
        bool is_comment = StrUtils::startswith(line, "#");

        if (line_match_regex.indexIn(line) != -1 &&
            (retval.match_line.size() == 0 || (int32_t(is_comment) < int32_t(retval.is_match_comment))))
        {
            retval.match_pos = retval.content.size();
            retval.match_line = line;
            retval.is_match_comment = is_comment;
        }

        retval.content.append(line);
        retval.content.push_back('\n');
    }

    KLOG_DEBUG("match line: %s is comment: %d.", retval.match_line.toLocal8Bit().data(), retval.is_match_comment);
    file.close();
    return retval;
}

PAM::MatchLineInfo PAM::addBehind(const QString &fallback_line, const QString &next_line_match_regex)
{
    MatchLineInfo retval;

    QFile file(this->conf_path_);

    if (!file.open(QIODevice::OpenModeFlag::ReadOnly))
    {
        KLOG_WARNING() << "open file fail, error is " << file.errorString() << "path is " << conf_path_;
        return retval;
    }
    auto contents = file.readAll();
    auto lines = StrUtils::splitLines(contents);
    QRegExp line_match_regex(next_line_match_regex);

    // 寻找匹配行，如果没有匹配的非注释行可用，则使用匹配的注释行（注释将被去掉）
    for (const auto &line : lines)
    {
        QVector<QString> fields;

        if (line_match_regex.indexIn(line) != -1 &&
            (retval.match_line.size() == 0))
        {
            retval.content.append(fallback_line);
            retval.content.push_back('\n');
            retval.match_pos = retval.content.size();
            retval.match_line = line;
        }

        retval.content.append(line);
        retval.content.push_back('\n');
    }

    KLOG_DEBUG("match line: %s is comment: %d.", retval.match_line.toLocal8Bit().data(), retval.is_match_comment);
    file.close();
    return retval;
}

bool PAM::writeToFile(const QString &content)
{
    KLOG_DEBUG("New contents: %s.", content.toLocal8Bit().data());
    return FileUtils::writeContents(this->conf_path_, content);
}

bool PAM::isWhitespaceInTail(const QString &str)
{
    RETURN_VAL_IF_TRUE(str.isEmpty(), false);
    return (isspace(str.at(str.length() - 1).toLatin1()) != 0);
}

}  // namespace Config

}  // namespace KS
