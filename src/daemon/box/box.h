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

#include <QObject>
#include <QStringList>
#include "src/daemon/box/box-dao.h"
#include "src/daemon/box/ecryptfs.h"

namespace KS
{
class Box : public QObject
{
    Q_OBJECT
public:
    Box(const QString &name,
        const QString &password,
        uint userUID,
        const QString &boxID = "",
        QObject *parent = nullptr);
    virtual ~Box(){};

    QString getBoxID();
    QString getBoxName();
    QString getUser();
    uint getUserUid();
    QString getPassphrase();
    QString retrievePassword(const QString &passphrase);
    bool delBox(const QString &currentPassword);
    bool mounted();
    bool mount(const QString &currentPassword);
    bool umount();
    bool modifyBoxPassword(const QString &currentPassword, const QString &newPassword);
    // 新建保险箱才需执行createBox，获取数据库中的保险箱无需进行此操作
    bool createBox();
    void initBoxMountStatus();
    bool mkdirDataDir();

private:
    QString getRandBoxUid();
    QString getRandStr(uint length);
    BoxRecord getBoxInfo();

private:
    QString m_name;
    QString m_boxID;
    QString m_password;
    uint m_userUID;

    BoxDao *m_boxDao;

    EcryptFS *m_ecryptFS;
};
}  // namespace KS
