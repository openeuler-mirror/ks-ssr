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
    KLOG_INFO("startMonitor.");

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

void ResourceMonitor::getSystemSpace(QString path)
{
    // 用于获取磁盘剩余空间
    struct statfs diskInfo;
    statfs(path.toLatin1(), &diskInfo);
    unsigned long long blocksize = diskInfo.f_bsize;               // 每个block里包含的字节数
    unsigned long long totalsize = blocksize * diskInfo.f_blocks;  // 总的字节数，f_blocks为block的数目

    unsigned long long freeDisk = diskInfo.f_bfree * blocksize;  // 剩余空间的大小

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
    //    unsigned long long availableDisk            = diskInfo.f_bavail * blocksize; 	//可用空间大小
    //	printf("Disk_free = %llu MB                 = %llu GB\nDisk_available = %llu MB = %llu GB\n",
    //
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

QVector<QString> ResourceMonitor::getVmStatS()
{
    QString results = run_cmd("vmstat");

    QVector<QString> line_list = stringSplit(results, "\n");  // std::string(results.c_str()).split("\n").at(2).split(" ");
    QVector<QString> r_list = stringSplit(line_list.at(2));

    QVector<QString> result_list = {};
    int i = 0;
    for (auto num : r_list)
    {
        if (num == "")
            continue;
        else
            i++;
        if (i == 7)  // si
            result_list.push_back(num);
        if (i == 8)  // so
            result_list.push_back(num);
    }
    //    KLOG_DEBUG("result_list si : %s so: %s",result_list.at(0).c_str(),result_list.at(1).c_str());
    return result_list;
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
    QString cpu_nums = run_cmd("grep 'model name' /proc/cpuinfo | wc -l");
    float simple_avg = avg / atoi(cpu_nums.toLatin1());
    return simple_avg;
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

    // this->cpuAverageLoadRatio_.emit(getCpuAverageLoad());
    Q_EMIT this->cpuAverageLoadRatio_(getCpuAverageLoad());

    // this->vmstatSiso_.emit(getVmStatS());
    Q_EMIT this->vmstatSiso_(getVmStatS());
    return true;
}

}  // namespace BRDaemon
}  // namespace KS
