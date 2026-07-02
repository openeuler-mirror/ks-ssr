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

#ifndef BOX_H
#define BOX_H

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
    explicit Box(const QString &name,
                 const QString &password,
                 uint userUID,
                 const QString &boxId = "",
                 QObject *parent = nullptr);

    QString getBoxID();
    QString getBoxName();
    QString getUser();
    uint getUserUid();
    QString getPassphrase();
    bool retrievePassword(const QString &passphrase, const QString &newPassword);
    bool delBox(const QString &inputPassword);
    bool isMount();
    bool mount(const QString &inputPassword);
    void umount();
    bool modifyBoxPassword(const QString &inputPassword, const QString &newPassword);

private:
    void init();

    QString getRandBoxUid();
    QString getRandStr(uint length);
    BoxRecord getBoxInfo();

private:
    QString m_name;
    QString m_boxId;
    QString m_password;
    uint m_userUID;

    BoxDao *m_boxDao;

    EcryptFS *m_ecryptFS;
};
}  // namespace KS
#endif  // BOX_H
