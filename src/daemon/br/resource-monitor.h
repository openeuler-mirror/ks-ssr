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
    ResourceMonitor();
    virtual ~ResourceMonitor();

    void startMonitor();
    void closeMonitor();

private:
    void getSystemSpace(QString path);  // 获取系统剩余空间
    float getMemoryRemainingRatio();    // 获取 vmstat si so
    float getCpuAverageLoad();          // 获取cpu负载

    bool monitorResource();

private:
    //    sigc::connection timeout_handler_;

    unsigned long long m_homeFreeSpace = 0;   //home 可用空间 MB
    unsigned long long m_homeTotalSpace = 0;  //home 总空间 MB
    unsigned long long m_rootFreeSpace = 0;   //根目录 可用空间 MB
    unsigned long long m_rootTotalSpace = 0;  //根目录 总空间 MB

signals:
    void homeFreeSpaceRatio_(const float &);
    void rootFreeSpaceRatio_(const float &);
    void cpuAverageLoadRatio_(const float &);
    void memoryRemainingRatio_(const float &);
};

}  // namespace BRDaemon
}  // namespace KS
