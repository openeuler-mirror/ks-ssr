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

#pragma once

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace KS
{
// 记录box信息
struct BoxRecord
{
    // box名字
    QString boxName;
    // box ID 作为box唯一标识
    QString boxID;
    // box 是否挂载
    bool mounted;
    // 加密的密码
    QString encryptpassword;
    // 加密后的口令
    QString encryptPassphrase;
    // 加密后的挂载身份验证令牌的签名，在执行装载之前，身份验证令牌必须位于内核密钥环中。
    QString encryptSig;
    // 调用者UID
    int userUID;
};

class BoxDao
{
public:
    BoxDao();
    ~BoxDao();

    // 添加box到数据库
    void addBox(const QString &boxName,
                const QString &boxID,
                bool mounted,
                const QString &encryptpassword,
                const QString &encryptPassphrase,
                const QString &encryptSig,
                int userUID);
    // 为box修改数据库中储存的挂载状态
    bool modifyMountStatus(const QString &boxID, bool mounted);
    // 为box修改数据库中储存的密码
    void modifyPasswd(const QString &boxID, const QString &encryptpassword);
    // 删除数据库中的box
    bool delBox(const QString &boxID);
    // 通过boxID获取box
    BoxRecord getBox(const QString &boxID);
    // 获取boxs表中所有box
    QList<BoxRecord> getBoxs();
    // 获取boxs表中的条数
    int getBoxCount();

private:
    void init();
    bool execute(const QString &cmd);

private:
    QSqlDatabase m_boxDb;
};
}  // namespace KS
