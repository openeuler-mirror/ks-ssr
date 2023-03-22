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

namespace KS
{
struct BoxInfo
{
    QString boxName;
    QString boxId;
    bool isMount;
    QString encryptpassword;
    QString encryptKey;
    QString encryptPspr;
    int senderUserUid;
};

class BoxDao
{
public:
    BoxDao();
    ~BoxDao();

    void addBox(const QString boxName, const QString boxId, bool isMount, const QString encryptpassword, const QString encryptKey, const QString encryptPspr, int senderUserUid);
    void modifyMountStatus(const QString boxId, bool isMount);
    void modifyPasswd(const QString boxId, const QString encryptpassword);
    bool delBox(const QString boxId);
    BoxInfo getBox(const QString boxId);
    QList<BoxInfo> getBoxs();
    int getBoxCount();

private:
    void init();

private:
    QSqlDatabase m_boxDb;
};
}  // namespace KS
#endif  // BOXDAO_H
