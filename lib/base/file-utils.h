/**
 * @file          /ks-ssr-manager/lib/base/file-utils.h
 * @brief         
 * @author        tangjie02 <tangjie02@kylinos.com.cn>
 * @copyright (c) 2020~2021 KylinSec Co., Ltd. All rights reserved. 
 */

#pragma once

#include "lib/base/base.h"

namespace KS
{
typedef sigc::slot<void, const Glib::RefPtr<Gio::File> &, const Glib::RefPtr<Gio::File> &, Gio::FileMonitorEvent> FileMonitorCallBack;

class FileUtils
{
public:
    FileUtils(){};
    virtual ~FileUtils(){};

    static Glib::RefPtr<Gio::FileMonitor> make_monitor(const std::string &path,
                                                       const FileMonitorCallBack &callback,
                                                       Gio::FileMonitorFlags flags = Gio::FILE_MONITOR_NONE);

    static Glib::RefPtr<Gio::FileMonitor> make_monitor_file(const std::string &path,
                                                            const FileMonitorCallBack &callback,
                                                            Gio::FileMonitorFlags flags = Gio::FILE_MONITOR_NONE);

    static Glib::RefPtr<Gio::FileMonitor> make_monitor_directory(const std::string &path,
                                                                 const FileMonitorCallBack &callback,
                                                                 Gio::FileMonitorFlags flags = Gio::FILE_MONITOR_NONE);

    // 获取文件内容，该函数会添加文件锁
    static bool read_contents_with_lock(const std::string &path, std::string &contents);
    // 写入文件内容，该函数会添加文件锁
    static bool write_contents_with_lock(const std::string &path, const std::string &contents);
    // Glib::file_set_contents调用了rename函数，这里使用write函数写入内容到文件避免产生文件删除事件
    static bool write_contents(const std::string &path, const std::string &contents);
};

}  // namespace KS