#pragma once

#include <qt5-log-i.h>
#include <sqlcipher/sqlite3.h>
#include <QDir>
#include <QString>
#include <QTextStream>
#include <QVariant>
#include <QVector>
#include "config.h"

#define PLAINTEXT_DB_PATH SSR_INSTALL_DATADIR "/ssr.dat"
#define ENCRYPTED_DB_PATH SSR_INSTALL_DATADIR "/ssr.db"
#define SQLCIPHER_ENCRYPT_PASSWD "123123"

namespace KS
{
using SqlRowDataType = QVector<QVariant>;
using SqlDataType = QVector<SqlRowDataType>;

class Database
{
public:
    Database();
    virtual ~Database();
    bool exec(const QString& cmd, SqlDataType* const result = nullptr);

private:
    inline bool checkExec(const int rc, const QString& action) const
    {
        if (SQLITE_OK == rc)
        {
            return !rc;
        }
        QString logMsg{};
        QTextStream logMsgStream(&logMsg);
        logMsgStream << "Failed to \"" << action
                     << "\", error number: " << rc
                     << ", error message: " << sqlite3_errmsg(m_db);
        logMsgStream.flush();
        KLOG_ERROR() << logMsg;
        return !rc;
    }

private:
    sqlite3* m_db;
};

};  // namespace KS