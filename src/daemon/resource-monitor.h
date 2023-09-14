#ifndef RESOURCEMONITOR_H
#define RESOURCEMONITOR_H

#pragma once

#include "lib/base/base.h"

namespace KS
{
namespace Daemon
{

class ResourceMonitor
{
public:
    ResourceMonitor();
    virtual ~ResourceMonitor();

    void startMonitor();
    void closeMonitor();

    sigc::signal<void, const float &> &signal_home_free_space_ratio() { return this->home_free_space_ratio_; };
    sigc::signal<void, const float &> &signal_root_free_space_ratio() { return this->root_free_space_ratio_; };
    sigc::signal<void, const float &> &signal_cpu_average_load_ratio() { return this->cpu_average_load_ratio_; };
    sigc::signal<void, const std::vector<std::string> &> &signal_vmstat_siso() { return this->vmstat_siso_; };

private:
    void getSystemSpace(std::string path); // 获取系统剩余空间
    std::vector<std::string> getVmStatS(); // 获取 vmstat si so
    float getCpuAverageLoad(); // 获取cpu负载

    bool monitorResource();

private:
//    sigc::connection timeout_handler_;

    unsigned long long m_homeFreeSpace = 0; //home 可用空间 MB
    unsigned long long m_homeTotalSpace = 0; //home 总空间 MB
    unsigned long long m_rootFreeSpace = 0; //根目录 可用空间 MB
    unsigned long long m_rootTotalSpace = 0; //根目录 总空间 MB

    sigc::signal<void, const float &> home_free_space_ratio_;
    sigc::signal<void, const float &> root_free_space_ratio_;
    sigc::signal<void, const float &> cpu_average_load_ratio_;
    sigc::signal<void, const std::vector<std::string> &> vmstat_siso_;
};

}  // namespace Daemon
}  // namespace KS

#endif // RESOURCEMONITOR_H
