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

namespace KS
{
BoxDao::BoxDao()
{
    this->init();
}

BoxDao::~BoxDao()
{
}

void BoxDao::addBox(const QString &boxName, const QString &boxId, bool isMount, const QString &encryptpassword, const QString &encryptKey, const QString &encryptPspr, int senderUserUid)
{
    KLOG_DEBUG() << "Insert sql table boxs. boxId = " << boxId;
    QSqlQuery query(m_boxDb);
    QString cmd = QString("insert into boxs (boxName,boxId,isMount,encryptpassword,encryptKey,encryptPspr,senderUserUid) values('%1','%2','%3','%4','%5','%6',%7);")
                      .arg(boxName, boxId, QString::number(isMount), encryptpassword, encryptKey, encryptPspr, QString::number(senderUserUid));

    if (!query.exec(cmd))
        KLOG_ERROR() << "Insert sql error!";
}

void BoxDao::modifyMountStatus(const QString &boxId, bool isMount)
{
    KLOG_DEBUG() << "Modify mount status to " << isMount;
    QSqlQuery query(m_boxDb);
    QString cmd = QString("update boxs set isMount ='%1' where boxId='%2';").arg(QString::number(isMount), boxId);
    if (!query.exec(cmd))
        KLOG_ERROR() << "Update boxs mount status error! boxId = " << boxId;
}

void BoxDao::modifyPasswd(const QString &boxId, const QString &encryptpassword)
{
    KLOG_DEBUG() << "Modify password. boxId = " << boxId;
    QSqlQuery query(m_boxDb);
    QString cmd = QString("update boxs set encryptpassword='%1' where boxId='%2';").arg(encryptpassword, boxId);
    if (!query.exec(cmd))
        KLOG_ERROR() << "ModifyQueryPasswd error!";
}

bool BoxDao::delBox(const QString &boxId)
{
    KLOG_DEBUG() << "Delete box. boxId = " << boxId;
    QSqlQuery query(m_boxDb);

    QString cmd = QString("delete from boxs where boxId='%1';").arg(boxId);
    if (!query.exec(cmd))
    {
        KLOG_ERROR() << "Delete box error! boxId = " << boxId;
        return false;
    }
    else
        return true;
}

BoxInfo BoxDao::getBox(const QString &boxId)
{
    QSqlQuery query(m_boxDb);
    BoxInfo boxInfo;
    QString cmd = "select * from boxs;";
    if (!query.exec(cmd))
        KLOG_WARNING() << "Box no found! boxId = " << boxId;
    else
    {
        while (query.next())
        {
            if (query.value(1).toString() == boxId)
            {
                boxInfo.boxName = query.value(0).toString();
                boxInfo.boxId = query.value(1).toString();
                boxInfo.isMount = QVariant(query.value(2).toString()).toBool();
                boxInfo.encryptpassword = query.value(3).toString();
                boxInfo.encryptKey = query.value(4).toString();
                boxInfo.encryptPspr = query.value(5).toString();
                boxInfo.senderUserUid = query.value(6).toInt();
                return boxInfo;
            }
        }
    }
    return boxInfo;
}

QList<BoxInfo> BoxDao::getBoxs()
{
    QSqlQuery query(m_boxDb);
    QList<BoxInfo> boxInfoList;

    QString cmd = "select * from boxs;";
    if (!query.exec(cmd))
        KLOG_WARNING() << "Table boxs is empty search error!";
    else
    {
        while (query.next())
        {
            BoxInfo boxInfo;
            boxInfo.boxName = query.value(0).toString();
            boxInfo.boxId = query.value(1).toString();
            boxInfo.isMount = QVariant(query.value(2).toString()).toBool();
            boxInfo.encryptpassword = query.value(3).toString();
            boxInfo.encryptKey = query.value(4).toString();
            boxInfo.encryptPspr = query.value(5).toString();
            boxInfo.senderUserUid = query.value(6).toInt();
            boxInfoList << boxInfo;
        }
    }
    return boxInfoList;
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
        KLOG_DEBUG() << "Boxs count = " << query.value(0).toInt();
        return query.value(0).toInt();
    }
}

void BoxDao::init()
{
    m_boxDb = QSqlDatabase::addDatabase("QSQLITE");
    m_boxDb.setDatabaseName(SC_INSTALL_DATADIR "/sc.dat");  //设置数据库名称
    if (!m_boxDb.open())
        KLOG_ERROR() << "BoxDao open error：" << m_boxDb.lastError();

    // 创建表
    QSqlQuery query(m_boxDb);

    //    if (!m_boxDb.isValid())
    if (!sqlTableExist(query))
    {
        query.exec("drop table boxs");

        QString cmd = "create table boxs(\
                                        boxName varchar(100),\
                                        boxId varchar(50),\
                                        isMount varchar(50),\
                                        encryptpassword varchar(1000),\
                                        encryptKey varchar(1000),\
                                        encryptPspr varchar(1000),\
                                        senderUserUid integer)";

        if (!query.exec(cmd))
            KLOG_ERROR() << "BoxDao query create error!";
    }
}

bool BoxDao::sqlTableExist(QSqlQuery query)
{
    //    QString cmd = QString("select count(*) from sqlite_master where type='table' and name='%1'").arg(tablename);
    if (!query.exec("select count() from boxs;"))
    {
        //        KLOG_DEBUG()<<"sqlTableExist error!";
        KLOG_WARNING() << "Table boxs is not exist!";
        return false;
    }
    else
    {
        query.seek(0);
        KLOG_DEBUG() << "Table boxs count is " << query.value(0).toInt();
        if (query.value(0).toInt() != 0)
            return true;
        else
            return false;
    }
}
}  // namespace KS
