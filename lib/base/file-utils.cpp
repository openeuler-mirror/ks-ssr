/**
 * @file          /ks-ssr-manager/lib/base/file-utils.cpp
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#include "lib/base/file-utils.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "lib/base/file-lock.h"

namespace KS
{
Glib::RefPtr<Gio::FileMonitor> FileUtils::make_monitor(const std::string &path,
                                                       const FileMonitorCallBack &callback,
                                                       Gio::FileMonitorFlags flags)
{
    auto file = Gio::File::create_for_path(path);
    try
    {
        auto monitor = file->monitor(flags);
        monitor->signal_changed().connect(callback);
        return monitor;
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Unable to monitor %s: %s", path.c_str(), e.what().c_str());
    }

    return Glib::RefPtr<Gio::FileMonitor>();
}

Glib::RefPtr<Gio::FileMonitor> FileUtils::make_monitor_file(const std::string &path,
                                                            const FileMonitorCallBack &callback,
                                                            Gio::FileMonitorFlags flags)
{
    auto file = Gio::File::create_for_path(path);
    try
    {
        auto monitor = file->monitor_file(flags);
        monitor->signal_changed().connect(callback);
        return monitor;
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Unable to file monitor %s: %s", path.c_str(), e.what().c_str());
    }

    return Glib::RefPtr<Gio::FileMonitor>();
}

Glib::RefPtr<Gio::FileMonitor> FileUtils::make_monitor_directory(const std::string &path,
                                                                 const FileMonitorCallBack &callback,
                                                                 Gio::FileMonitorFlags flags)
{
    auto file = Gio::File::create_for_path(path);
    try
    {
        auto monitor = file->monitor_directory(flags);
        monitor->signal_changed().connect(callback);
        return monitor;
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Unable to directory monitor %s: %s", path.c_str(), e.what().c_str());
    }

    return Glib::RefPtr<Gio::FileMonitor>();
}

bool FileUtils::read_contents_with_lock(const std::string &path, std::string &contents)
{
    auto file_lock = FileLock::create_share_lock(path, O_RDONLY, 0);
    if (!file_lock)
    {
        KLOG_DEBUG("Failed to create share lock for %s.", path.c_str());
        return false;
    }

    try
    {
        contents = Glib::file_get_contents(path);
    }
    catch (const Glib::FileError &e)
    {
        KLOG_WARNING("Failed to get file contents: %s.", path.c_str());
        return false;
    }
    return true;
}

bool FileUtils::write_contents_with_lock(const std::string &path, const std::string &contents)
{
    auto file_lock = FileLock::create_excusive_lock(path, O_RDWR | O_CREAT | O_SYNC, CONF_FILE_PERMISSION);
    if (!file_lock)
    {
        KLOG_WARNING("Failed to lock file %s.", path.c_str());
        return false;
    }

    try
    {
        KLOG_DEBUG("Write contents: %s.", contents.c_str());
        FileUtils::write_contents(path, contents);
    }
    catch (const Glib::Error &e)
    {
        KLOG_WARNING("Failed to write file contents: %s.", path.c_str());
        return false;
    }
    return true;
}

bool FileUtils::write_contents(const std::string &path, const std::string &contents)
{
    KLOG_PROFILE("path: %s", path.c_str());

    int fp = -1;

    SSR_SCOPE_EXIT({
        if (fp > 0)
        {
            close(fp);
        }
    });

    fp = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);

    if (fp < 0)
    {
        KLOG_WARNING("Failed to open file %s: %s.", path.c_str(), strerror(errno));
        return false;
    }

    if (write(fp, contents.c_str(), contents.length()) < 0)
    {
        KLOG_WARNING("Failed to write file %s: %s.", path.c_str(), strerror(errno));
        return false;
    }

    return true;
}
}  // namespace KS
