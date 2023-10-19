#include "configuration.h"

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