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

#include "resource-monitor.h"
#include <sys/statfs.h>

namespace KS
{
namespace BRDaemon
{
ResourceMonitor::ResourceMonitor()
{
}

ResourceMonitor::~ResourceMonitor()
{
}

void ResourceMonitor::startMonitor()
{
    KLOG_DEBUG("startMonitor.");

    monitorResource();
}

void ResourceMonitor::closeMonitor()
{
    //    KLOG_DEBUG("closeMonitor.");
}

QString run_cmd(QString cmd)
{
    char line[300];
    QString result = "";
    FILE *fp;
    QString cmdPort = cmd;
    // 系统调用
    const char *sysCommand = cmdPort.toLatin1();
    // 如果没有打开端口
    if ((fp = popen(sysCommand, "r")) == NULL)
    {
        //	   cout << "error" << endl;
        return result;
    }
    // 如果端口号打开了
    while (fgets(line, sizeof(line) - 1, fp) != NULL)
    {
        //    	cout << line ;
        result.append(line);
    }

    pclose(fp);
    return result;
}

void ResourceMonitor::getSystemSpace(const QString &path)
{
    // 用于获取磁盘剩余空间
    struct statfs diskInfo;
    statfs(path.toLatin1(), &diskInfo);
    // 每个block里包含的字节数
    unsigned long long blocksize = diskInfo.f_bsize;
    // 总的字节数，f_blocks为block的数目
    unsigned long long totalsize = blocksize * diskInfo.f_blocks;
    // 剩余空间的大小
    unsigned long long freeDisk = diskInfo.f_bfree * blocksize;

    if (path == "/home")
    {
        m_homeTotalSpace = totalsize >> 20;
        m_homeFreeSpace = freeDisk >> 20;
    }
    else if (path == "/")
    {
        m_rootTotalSpace = totalsize >> 20;
        m_rootFreeSpace = freeDisk >> 20;
    }
}

QVector<QString> stringSplit(const QString &s, const QString &delim = " ")
{
    QVector<QString> elems;
    size_t pos = 0;
    size_t len = s.length();
    size_t delim_len = delim.length();
    if (delim_len == 0) return elems;
    while (pos < len)
    {
        int find_pos = s.indexOf(delim, pos);
        if (find_pos < 0)
        {
            elems.push_back(s.mid(pos, len - pos));
            break;
        }
        elems.push_back(s.mid(pos, find_pos - pos));
        pos = find_pos + delim_len;
    }
    return elems;
}

float ResourceMonitor::getMemoryRemainingRatio()
{
    char memTotal[20] = "", memFree[20] = "", memAvailable[20] = "", cached[20] = "", buffers[20] = "";

    FILE *file = fopen("/proc/meminfo", "r");
    if (file == nullptr)
    {
        KLOG_ERROR("cannot open /proc/meminfo");
        return -1;
    }
    fscanf(file, "MemTotal: %s kB\n", memTotal);
    fscanf(file, "MemFree: %s kB\n", memFree);
    fscanf(file, "MemAvailable: %s kB\n", memAvailable);
    fscanf(file, "Buffers: %s kB\n", buffers);
    fscanf(file, "Cached: %s kB\n", cached);
    fclose(file);
    return (atof(memFree) + atof(cached) + atof(buffers)) / atof(memTotal);
}

float ResourceMonitor::getCpuAverageLoad()
{
    char buff[20];
    float avg = 0.00;
    FILE *fd = fopen("/proc/loadavg", "r");
    if (fd != NULL)
    {
        fgets(buff, sizeof(buff), fd);
        // 程序读取五分钟的平均负载
        sscanf(&buff[5], "%f", &avg);
        fclose(fd);
    }
    auto cpu_nums = run_cmd("grep 'model name' /proc/cpuinfo | wc -l");
    return avg / atoi(cpu_nums.toLatin1());
}

bool ResourceMonitor::monitorResource()
{
    getSystemSpace("/home");
    float homeRatio = float(m_homeFreeSpace) / float(m_homeTotalSpace);
    // this->homeFreeSpaceRatio_.emit(homeRatio);
    Q_EMIT this->homeFreeSpaceRatio_(homeRatio);
    getSystemSpace("/");
    float rootRatio = float(m_rootFreeSpace) / float(m_rootTotalSpace);
    // this->rootFreeSpaceRatio_.emit(rootRatio);
    Q_EMIT this->rootFreeSpaceRatio_(rootRatio);
    Q_EMIT this->cpuAverageLoadRatio_(getCpuAverageLoad());
    Q_EMIT this->memoryRemainingRatio_(getMemoryRemainingRatio());
    return true;
}

}  // namespace BRDaemon
}  // namespace KS
