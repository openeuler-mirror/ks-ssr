#include "database.h"

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
    auto callback = [](void* callback_arg, int argc, char** argv, char** azColName) -> int {
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

};  // namespace KS