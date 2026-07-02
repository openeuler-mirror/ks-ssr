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

    // 添加box到数据库
    void addBox(const QString &boxName, const QString &boxId, bool isMount, const QString &encryptpassword, const QString &encryptKey, const QString &encryptPspr, int senderUserUid);
    // 为box修改数据库中储存的挂载状态
    void modifyMountStatus(const QString &boxId, bool isMount);
    // 为box修改数据库中储存的密码
    void modifyPasswd(const QString &boxId, const QString &encryptpassword);
    // 删除数据库中的box
    bool delBox(const QString &boxId);
    // 通过boxId获取box
    BoxInfo getBox(const QString &boxId);
    // 获取boxs表中所有box
    QList<BoxInfo> getBoxs();
    // 获取boxs表中的条数
    int getBoxCount();

private:
    void init();
    // 判断表格是否存在
    bool sqlTableExist(QSqlQuery query);

private:
    QSqlDatabase m_boxDb;
};
}  // namespace KS
#endif  // BOXDAO_H
