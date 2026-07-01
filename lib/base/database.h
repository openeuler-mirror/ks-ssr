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

#include <qt5-log-i.h>
#include <sqlcipher/sqlite3.h>

class QVariant;
template <typename T> class QVector;

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
    bool checkExec(const int rc, const QString& action) const;

private:
    sqlite3* m_db;
};

};  // namespace KS