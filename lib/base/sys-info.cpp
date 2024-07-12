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
 * Author:     yangfeng <yangfeng@kylinsec.com.cn>
 */

#include "sys-info.h"
#include <QNetworkInterface>

QString getIPPath()
{
    QString strIpAddress;
    auto ipAddressesList = QNetworkInterface::allAddresses();

    // 获取第一个本主机的IPv4地址
    auto nListSize = ipAddressesList.size();
    for (int i = 0; i < nListSize; ++i)
    {
        if (ipAddressesList.at(i) == QHostAddress::LocalHost || !ipAddressesList.at(i).toIPv4Address())
        {
            continue;
        }
        strIpAddress = ipAddressesList.at(i).toString();
        break;
    }

    // 如果没有找到，则以本地IP地址为IP
    if (strIpAddress.isEmpty())
    {
        strIpAddress = QHostAddress(QHostAddress::LocalHost).toString();
    }
    return strIpAddress;
}

QString getMacPath()
{
    auto nets = QNetworkInterface::allInterfaces();  // 获取所有网络接口列表
    auto nCnt = nets.count();
    QString strMacAddr = "";
    for (int i = 0; i < nCnt; i++)
    {
        // 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
        if (nets[i].flags().testFlag(QNetworkInterface::IsUp) && nets[i].flags().testFlag(QNetworkInterface::IsRunning) && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            strMacAddr = nets[i].hardwareAddress();
            break;
        }
    }

    return strMacAddr;
}

QString getKernelInfo()
{
    return QSysInfo::kernelType() + QSysInfo::kernelVersion();
}
