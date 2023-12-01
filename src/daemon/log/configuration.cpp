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
#include <QSettings>

bool KS::Log::Configurations::isValid()
{
    if ((m_maxLogFile < 5 || m_maxLogFile >= 50) ||
        (m_numLogs < 1 || m_numLogs >= 10) ||
        (m_ip.isNull()))
    {
        return false;
    }
    return true;
}

KS::Log::Configurations::Configurations(const QString& configPath)
    : m_config(new QSettings(configPath, QSettings::NativeFormat, this)),
      m_maxLogFile(m_config->value("log/max_log_file", 8).toUInt()),
      m_numLogs(m_config->value("log/num_logs", 5).toUInt()),
      m_account(m_config->value("log/account").toString()),
      m_passwd(m_config->value("log/passwd").toString())
{
    m_ip.setAddress(m_config->value("ip").toString());
}

KS::Log::Configurations::~Configurations()
{
}

void KS::Log::Configurations::operator=(const Configurations& other)
{
    m_maxLogFile = other.m_maxLogFile;
    m_numLogs = other.m_numLogs;
    m_account = other.m_account;
    m_passwd = other.m_passwd;
    m_ip = other.m_ip;
}