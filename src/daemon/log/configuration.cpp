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

#include "src/daemon/log/configuration.h"
#include <qt5-log-i.h>
#include <QSettings>

#define LOG_FILE_LINE_MIN_VALUE 3000
#define LOG_FILE_LINE_MAX_VALUE 500000
#define LOG_FILE_NUM_MIN_VALUE 1
#define LOG_FILE_NUM_MAX_VALUE 10

KS::Log::Configurations::Configurations(const QString& configPath)
    : m_config(new QSettings(configPath, QSettings::NativeFormat, this)),
      //   m_maxLogFileLine(m_config->value("log/max_log_file", m_maxLogFileLineDefaultValue).toUInt())
      m_account(m_config->value("log/account").toString()),
      m_passwd(m_config->value("log/passwd").toString()),
      m_remotePath(m_config->value("log/remotePath").toString())
{
    auto numLogs = m_config->value("log/numLogs", m_numLogsDefaultValue).toUInt();
    if (numLogs >= LOG_FILE_NUM_MIN_VALUE || numLogs <= LOG_FILE_NUM_MAX_VALUE)
    {
        m_numLogs = numLogs;
    }
    else
    {
        KLOG_WARNING() << "NumLogs must less than 11 and greater than 0";
    }
    m_ip.setAddress(m_config->value("log/ip").toString());
    KLOG_INFO() << "MaxLogFileLine value is: " << m_maxLogFileLine
                << ", NumLogs value is: " << m_numLogs
                << ", remote ip is: " << m_ip
                << ", remote path is: " << m_remotePath;
}

KS::Log::Configurations::~Configurations()
{
}

void KS::Log::Configurations::operator=(const Configurations& other)
{
    // m_maxLogFileLine = other.m_maxLogFileLine;
    m_numLogs = other.m_numLogs;
    m_account = other.m_account;
    m_passwd = other.m_passwd;
    m_remotePath = other.m_remotePath;
    m_ip = other.m_ip;
}