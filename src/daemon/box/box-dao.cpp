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
#include "include/sc-marcos.h"

namespace KS
{
BoxDao::BoxDao()
{
    this->init();
}

BoxDao::~BoxDao()
{
}

void BoxDao::addBox(const QString &boxName,
                    const QString &boxId,
                    bool isMount,
                    const QString &encryptpassword,
                    const QString &encryptPspr,
                    const QString &encryptSig,
                    int userUid)
{
    QString cmd = QString("insert into boxs (boxName,boxId,isMount,encryptpassword,encryptPspr,encryptSig,userUid) \
                      values('%1','%2','%3','%4','%5','%6',%7);")
                      .arg(boxName, boxId, QString::number(isMount), encryptpassword, encryptPspr, encryptSig, QString::number(userUid));

    this->execute(cmd);
}

void BoxDao::modifyMountStatus(const QString &boxId, bool isMount)
{
    QString cmd = QString("update boxs set isMount ='%1' where boxId='%2';").arg(QString::number(isMount), boxId);

    this->execute(cmd);
}

void BoxDao::modifyPasswd(const QString &boxId, const QString &encryptpassword)
{
    QString cmd = QString("update boxs set encryptpassword='%1' where boxId='%2';").arg(encryptpassword, boxId);

    this->execute(cmd);
}

bool BoxDao::delBox(const QString &boxId)
{
    QString cmd = QString("delete from boxs where boxId='%1';").arg(boxId);

    RETURN_VAL_IF_TRUE(!this->execute(cmd), false)

    return true;
}

BoxRecord BoxDao::getBox(const QString &boxId)
{
    auto boxs = getBoxs();
    for (auto box : boxs)
    {
        RETURN_VAL_IF_TRUE(box.boxId == boxId, box)
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
            boxRecord.boxId = query.value(1).toString();
            boxRecord.isMount = QVariant(query.value(2).toString()).toBool();
            boxRecord.encryptpassword = query.value(3).toString();
            boxRecord.encryptPspr = query.value(4).toString();
            boxRecord.encryptSig = query.value(5).toString();
            boxRecord.userUid = query.value(6).toInt();
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
    m_boxDb.setDatabaseName(SC_INSTALL_DATADIR "/sc.dat");  //设置数据库名称
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
                                        boxId varchar(50),\
                                        isMount varchar(50),\
                                        encryptpassword varchar(1000),\
                                        encryptPspr varchar(1000),\
                                        encryptSig varchar(1000),\
                                        userUid integer)";

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
