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

#include "lib/base/base.h"

namespace KS
{
namespace BRDaemon
{
class ResourceMonitor : public QObject
{
    Q_OBJECT
public:
    ResourceMonitor(QObject *parent);
    virtual ~ResourceMonitor();

    void startMonitor();
    void closeMonitor();

private:
    // 获取系统剩余空间
    void getSystemSpace(const QString &path);
    // 获取 vmstat si so
    float getMemoryRemainingRatio();
    // 获取cpu负载
    float getCpuAverageLoad();

    bool monitorResource();

private:
    // home 可用空间 MB
    unsigned long long m_homeFreeSpace = 0;
    // home 总空间 MB
    unsigned long long m_homeTotalSpace = 0;
    // 根目录 可用空间 MB
    unsigned long long m_rootFreeSpace = 0;
    // 根目录 总空间 MB
    unsigned long long m_rootTotalSpace = 0;

signals:
    void homeFreeSpaceRatio_(float);
    void rootFreeSpaceRatio_(float);
    void cpuAverageLoadRatio_(float);
    void memoryRemainingRatio_(float);
};

}  // namespace BRDaemon
}  // namespace KS
