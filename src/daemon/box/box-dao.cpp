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

void BoxDao::addBox(const QString boxName, const QString boxId, bool isMount, const QString encryptpassword, const QString encryptKey, const QString encryptPspr, int senderUserUid)
{
    QSqlQuery query(m_boxDb);

    QString cmd = QString("insert into boxs (boxName,boxId,isMount,encryptpassword,encryptKey,encryptPspr,senderUserUid) values('%1','%2','%3','%4','%5','%6',%7);")
                      .arg(boxName, boxId, QString::number(isMount), encryptpassword, encryptKey, encryptPspr, QString::number(senderUserUid));
    KLOG_DEBUG() << "addQuery cmd = " << cmd;
    if (!query.exec(cmd))
        KLOG_DEBUG() << "BoxDao insert error!";
}

void BoxDao::modifyMountStatus(const QString boxId, bool isMount)
{
    QSqlQuery query(m_boxDb);

    QString cmd = QString("update boxs set isMount ='%1' where boxId='%2';").arg(QString::number(isMount), boxId);
    KLOG_DEBUG() << "fixedQueryMountStatus cmd = " << cmd;
    if (!query.exec(cmd))
        KLOG_DEBUG() << "modifyQueryMountStatus error!";
}

void BoxDao::modifyPasswd(const QString boxId, const QString encryptpassword)
{
    QSqlQuery query(m_boxDb);

    QString cmd = QString("update boxs set encryptpassword='%1' where boxId='%2';").arg(encryptpassword, boxId);
    KLOG_DEBUG() << "fixedQueryPasswd cmd = " << cmd;
    if (!query.exec(cmd))
        KLOG_DEBUG() << "ModifyQueryPasswd error!";
}

bool BoxDao::delBox(const QString boxId)
{
    QSqlQuery query(m_boxDb);

    QString cmd = QString("delete from boxs where boxId='%1';").arg(boxId);
    KLOG_DEBUG() << "addQuery cmd = " << cmd;
    if (!query.exec(cmd))
    {
        KLOG_DEBUG() << "BoxDao delete error! boxId = " << boxId;
        return false;
    }
    else
        return true;
}

BoxInfo BoxDao::getBox(const QString boxId)
{
    QSqlQuery query(m_boxDb);
    BoxInfo boxInfo;
    QString cmd = "select * from boxs;";
    if (!query.exec(cmd))
        KLOG_DEBUG() << "select error!";
    else
    {
        while (query.next())
        {
            if (query.value(1).toString() == boxId)
            {
                boxInfo.boxName = query.value(0).toString();
                boxInfo.boxId = query.value(1).toString();
                boxInfo.isMount = query.value(2).toBool();
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
        KLOG_DEBUG() << "select error!";
    else
    {
        while (query.next())
        {
            BoxInfo boxInfo;
            boxInfo.boxName = query.value(0).toString();
            boxInfo.boxId = query.value(1).toString();
            boxInfo.isMount = query.value(2).toBool();
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
        KLOG_DEBUG() << "getQuerySum error!";
        return -1;
    }
    else
    {
        query.seek(0);
        KLOG_DEBUG() << "count = " << query.value(0).toInt();
        return query.value(0).toInt();
    }
}

// 判断表格是否存在
bool sqlTableExist(QSqlQuery query)
{
    //    QString cmd = QString("select count(*) from sqlite_master where type='table' and name='%1'").arg(tablename);
    if (!query.exec("select count() from boxs;"))
    {
        //        KLOG_DEBUG()<<"sqlTableExist error!";
        KLOG_DEBUG() << "not Exist!";
        return false;
    }
    else
    {
        query.seek(0);
        KLOG_DEBUG() << "count = " << query.value(0).toInt();
        if (query.value(0).toInt() != 0)
            return true;
        else
            return false;
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
}  // namespace KS
