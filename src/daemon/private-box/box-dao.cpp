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
 * Author:     chendingjian <chendingjian@kylinos.com.cn>
 */

#include "box-dao.h"
#include <qt5-log-i.h>
#include "config.h"
#include "include/ssr-marcos.h"

namespace KS
{
namespace PrivateBox
{
BoxDao::BoxDao()
{
    init();
}

BoxDao::~BoxDao()
{
}

void BoxDao::addBox(const QString &boxName,
                    const QString &boxID,
                    bool mounted,
                    const QString &encryptpassword,
                    const QString &encryptPassphrase,
                    const QString &encryptSig,
                    int userUID)
{
    QString cmd = QString("insert into boxs (boxName,boxID,mounted,encryptpassword,encryptPassphrase,encryptSig,userUID) \
                      values('%1','%2','%3','%4','%5','%6',%7);")
                      .arg(boxName, boxID, QString::number(mounted), encryptpassword, encryptPassphrase, encryptSig, QString::number(userUID));

    execute(cmd);
}

bool BoxDao::modifyMountStatus(const QString &boxID, bool mounted)
{
    QString cmd = QString("update boxs set mounted ='%1' where boxID='%2';").arg(QString::number(mounted), boxID);

    return execute(cmd);
}

void BoxDao::modifyPasswd(const QString &boxID, const QString &encryptpassword)
{
    QString cmd = QString("update boxs set encryptpassword='%1' where boxID='%2';").arg(encryptpassword, boxID);

    execute(cmd);
}

bool BoxDao::delBox(const QString &boxID)
{
    QString cmd = QString("delete from boxs where boxID='%1';").arg(boxID);

    RETURN_VAL_IF_TRUE(!execute(cmd), false)

    return true;
}

BoxRecord BoxDao::getBox(const QString &boxID)
{
    auto boxs = getBoxs();
    for (auto box : boxs)
    {
        RETURN_VAL_IF_TRUE(box.boxID == boxID, box)
    }

    return BoxRecord();
}

QList<BoxRecord> BoxDao::getBoxs()
{
    QList<BoxRecord> boxRecordList;
    SqlDataType result{};

    QString cmd = "select * from boxs;";

    if (!m_boxDb->exec(cmd, &result))
    {
        KLOG_WARNING() << "Table boxs is empty search error!";
    }
    else
    {
        std::for_each(result.cbegin(), result.cend(), [&boxRecordList](const SqlRowDataType& row) {
            BoxRecord boxRecord;
            boxRecord.boxName = row[0].toString();
            boxRecord.boxID = row[1].toString();
            boxRecord.mounted = row[2].toBool();
            boxRecord.encryptpassword = row[3].toString();
            boxRecord.encryptPassphrase = row[4].toString();
            boxRecord.encryptSig = row[5].toString();
            boxRecord.userUID = row[6].toInt();
            boxRecordList << boxRecord;
        });
    }
    return boxRecordList;
}

int BoxDao::getBoxCount()
{
    QString cmd = "select count() from boxs;";
    SqlDataType result{};
    if (!m_boxDb->exec(cmd, &result))
    {
        KLOG_ERROR() << "Get boxs sum error!";
        return -1;
    }
    else
    {
        KLOG_DEBUG() << "The total number of boxes obtained is " << result[0][0].toInt();
        return result[0][0].toInt();
    }
}

void BoxDao::init()
{
    m_boxDb = new Database();

    // 表格不存在则创建
    if (getBoxCount() < 1)
    {
        m_boxDb->exec("drop table boxs");

        QString cmd = "create table boxs(\
                                        boxName varchar(100),\
                                        boxID varchar(50),\
                                        mounted varchar(50),\
                                        encryptpassword varchar(1000),\
                                        encryptPassphrase varchar(1000),\
                                        encryptSig varchar(1000),\
                                        userUID integer)";

        if (!m_boxDb->exec(cmd))
        {
            KLOG_ERROR() << "BoxDao query create error!";
        }
    }
}

bool BoxDao::execute(const QString &cmd)
{
    if (!m_boxDb->exec(cmd))
    {
        KLOG_ERROR() << "Execute sql query cmd error! cmd = " << cmd;
        return false;
    }

    return true;
}
}  // namespace Box
}  // namespace KS
