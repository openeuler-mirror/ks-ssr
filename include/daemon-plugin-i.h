/**
 * Copyright (c) 2024 ~ 2025 KylinSec Co., Ltd.
 * ks-ssr is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     tangjie02 <tangjie02@kylinsec.com.cn>
 */

#pragma once

#include <QtPlugin>

namespace KS
{
#define IDAEMON_IID "com.kylinsec.ssr.daemon.plugin"

class IDaemonPlugin
{
public:
    virtual ~IDaemonPlugin(){};

    // 插件是否可用，例如运行环境中依赖是否齐全，操作系统版本是否支持
    virtual bool isAvailable() = 0;
    // 激活插件
    virtual void activate() = 0;
    // 取消激活插件
    virtual void deactivate() = 0;
};
}  // namespace KS

Q_DECLARE_INTERFACE(KS::IDaemonPlugin, IDAEMON_IID)
