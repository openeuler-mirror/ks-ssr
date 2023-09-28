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

#include "box-dao.h"
#include <qt5-log-i.h>
#include "config.h"
#include "include/ksc-marcos.h"

namespace KS
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
    QSqlQuery query(m_boxDb);
    QList<BoxRecord> boxRecordList;

    QString cmd = "select * from boxs;";

    if (!query.exec(cmd))
    {
        KLOG_WARNING() << "Table boxs is empty search error!";
    }
    else
    {
        while (query.next())
        {
            BoxRecord boxRecord;
            boxRecord.boxName = query.value(0).toString();
            boxRecord.boxID = query.value(1).toString();
            boxRecord.mounted = QVariant(query.value(2).toString()).toBool();
            boxRecord.encryptpassword = query.value(3).toString();
            boxRecord.encryptPassphrase = query.value(4).toString();
            boxRecord.encryptSig = query.value(5).toString();
            boxRecord.userUID = query.value(6).toInt();
            boxRecordList << boxRecord;
        }
    }
    return boxRecordList;
}

int BoxDao::getBoxCount()
{
    QSqlQuery query(m_boxDb);
    QString cmd = "select count() from boxs;";
    if (!query.exec(cmd))
    {
        KLOG_ERROR() << "Get boxs sum error!";
        return -1;
    }
    else
    {
        query.seek(0);
        KLOG_DEBUG() << "The total number of boxes obtained is " << query.value(0).toInt();
        return query.value(0).toInt();
    }
}

void BoxDao::init()
{
    m_boxDb = QSqlDatabase::addDatabase("QSQLITE");
    m_boxDb.setDatabaseName(KSC_INSTALL_DATADIR "/ksc.dat");  //设置数据库名称
    if (!m_boxDb.open())
    {
        KLOG_ERROR() << "BoxDao open error ：" << m_boxDb.lastError();
    }

    // 创建表
    QSqlQuery query(m_boxDb);

    // 表格不存在则创建
    if (getBoxCount() < 1)
    {
        query.exec("drop table boxs");

        QString cmd = "create table boxs(\
                                        boxName varchar(100),\
                                        boxID varchar(50),\
                                        mounted varchar(50),\
                                        encryptpassword varchar(1000),\
                                        encryptPassphrase varchar(1000),\
                                        encryptSig varchar(1000),\
                                        userUID integer)";

        if (!query.exec(cmd))
        {
            KLOG_ERROR() << "BoxDao query create error!";
        }
    }
}

bool BoxDao::execute(const QString &cmd)
{
    QSqlQuery query(m_boxDb);

    if (!query.exec(cmd))
    {
        KLOG_ERROR() << "Execute sql query cmd error! cmd = " << cmd;
        return false;
    }

    return true;
}
}  // namespace KS
