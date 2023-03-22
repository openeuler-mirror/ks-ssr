/**
 * Copyright (c) 2023 ~ 2024 KylinSec Co., Ltd.
 * ks-sc is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2. 
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2 
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.  
 * See the Mulan PSL v2 for more details.  
 * 
 * Author:     chendingjian <chendingjian@kylinos.com.cn> 
 */

#ifndef BOXDAO_H
#define BOXDAO_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include "lib/base/base.h"

namespace KS
{
class BoxDao
{
public:
    BoxDao();
    ~BoxDao();

    static BoxDao *getInstance()
    {
        return m_instance;
    };

    static QSqlDatabase getInstanceDb()
    {
        return m_instance->m_boxDb;
    };

    static void globalInit()
    {
        m_instance = new BoxDao();
        m_instance->init();
    };

    static void globalDeinit() { delete m_instance; };

    void addQuery(const QString boxName, const QString boxId, bool isMount, const QString encryptpassword, const QString encryptKey, const QString encryptPspr, int senderUserUid);
    void modifyQueryMountStatus(const QString boxId, bool isMount);
    void ModifyQueryPasswd(const QString boxId, const QString encryptpassword);
    bool delQuery(const QString boxId);
    QSqlQuery findQuery(const QString boxId);
    int getQueryCount();

private:
    void init();

private:
    static BoxDao *m_instance;
    QSqlDatabase m_boxDb;
};
}  // namespace KS
#endif  // BOXDAO_H
