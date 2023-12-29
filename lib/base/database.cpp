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

#include "lib/base/database.h"
#include <sqlcipher/sqlite3.h>
#include <QDir>
#include <QTextStream>
#include <QVariant>
#include <QVector>
#include "config.h"

#define PLAINTEXT_DB_PATH SSR_INSTALL_DATADIR "/ssr.dat"
#define ENCRYPTED_DB_PATH SSR_INSTALL_DATADIR "/ssr.db"
#define SQLCIPHER_ENCRYPT_PASSWD "123123"

namespace KS
{
#pragma message("在 spec 中完成 ssr.dat 到 ssr.db 的迁移")
#pragma message("使用 \%config 标记数据库")
#warning "发布时记得修改密码"

Database::Database()
{
    QDir dir;
    int rc;
    rc = sqlite3_open_v2(ENCRYPTED_DB_PATH, &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (!checkExec(rc, "open encrypted db"))
    {
        abort();
    }
    rc = sqlite3_key(m_db, SQLCIPHER_ENCRYPT_PASSWD, sizeof(SQLCIPHER_ENCRYPT_PASSWD) - 1);
    if (!checkExec(rc, "decrypt db"))
    {
        abort();
    }
}

Database::~Database()
{
    sqlite3_close_v2(m_db);
}

bool Database::exec(const QString& cmd, SqlDataType* const result)
{
    int rc = 0;
    KLOG_DEBUG() << "Exec sql cmd: " << cmd;
    if (result == nullptr)
    {
        rc = sqlite3_exec(m_db, cmd.toLatin1(), nullptr, result, nullptr);
        return checkExec(rc, cmd);
    }
    auto callback = [](void* callback_arg, int argc, char** argv, char** azColName) -> int
    {
        auto ret = reinterpret_cast<SqlDataType*>(callback_arg);
        SqlRowDataType row;
        row.reserve(argc);
        for (auto i = 0; i < argc; i++)
        {
            row << argv[i];
        }
        ret->append(row);
        return SQLITE_OK;
    };
    rc = sqlite3_exec(m_db, cmd.toLatin1(), callback, result, nullptr);
    return checkExec(rc, cmd);
}

bool Database::checkExec(const int rc, const QString& action) const
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
};  // namespace KS