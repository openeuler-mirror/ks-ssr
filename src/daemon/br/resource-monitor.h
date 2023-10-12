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

#ifndef RESOURCEMONITOR_H
#define RESOURCEMONITOR_H

#pragma once

#include "lib/base/base.h"

namespace KS
{
namespace BRDaemon
{
class ResourceMonitor : public QObject
{
    Q_OBJECT
public:
    ResourceMonitor();
    virtual ~ResourceMonitor();

    void startMonitor();
    void closeMonitor();

    // sigc::signal<void, const float &> &signal_home_free_space_ratio() { return this->homeFreeSpaceRatio_; };
    // sigc::signal<void, const float &> &signal_root_free_space_ratio() { return this->rootFreeSpaceRatio_; };
    // sigc::signal<void, const float &> &signal_cpu_average_load_ratio() { return this->cpuAverageLoadRatio_; };
    // sigc::signal<void, const std::vector<std::string> &> &signal_vmstat_siso() { return this->vmstatSiso_; };

private:
    void getSystemSpace(QString path);  // 获取系统剩余空间
    QVector<QString> getVmStatS();      // 获取 vmstat si so
    float getCpuAverageLoad();          // 获取cpu负载

    bool monitorResource();

private:
    //    sigc::connection timeout_handler_;

    unsigned long long m_homeFreeSpace = 0;   //home 可用空间 MB
    unsigned long long m_homeTotalSpace = 0;  //home 总空间 MB
    unsigned long long m_rootFreeSpace = 0;   //根目录 可用空间 MB
    unsigned long long m_rootTotalSpace = 0;  //根目录 总空间 MB

    // sigc::signal<void, const float &> homeFreeSpaceRatio_;
    // sigc::signal<void, const float &> rootFreeSpaceRatio_;
    // sigc::signal<void, const float &> cpuAverageLoadRatio_;
    // sigc::signal<void, const std::vector<std::string> &> vmstatSiso_;

signals:
    void homeFreeSpaceRatio_(const float &);
    void rootFreeSpaceRatio_(const float &);
    void cpuAverageLoadRatio_(const float &);
    void vmstatSiso_(const QVector<QString>);
};

}  // namespace BRDaemon
}  // namespace KS

#endif  // RESOURCEMONITOR_H
