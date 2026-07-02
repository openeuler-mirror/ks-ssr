#include "box-dao.h"

namespace KS
{
BoxDao *BoxDao::m_instance = nullptr;

BoxDao::BoxDao()
{
}

BoxDao::~BoxDao()
{
}

void BoxDao::addQuery(const QString boxName, const QString boxId, bool isMount, const QString encryptpassword, const QString encryptKey, const QString encryptPspr, int senderUserUid)
{
    QSqlQuery query(m_boxDb);

    QString cmd = QString("insert into notes (boxName,boxId,isMount,encryptpassword,encryptKey,encryptPspr,senderUserUid) values('%1','%2','%3','%4','%5','%6',%7);")
                      .arg(boxName, boxId, QString::number(isMount), encryptpassword, encryptKey, encryptPspr, QString::number(senderUserUid));
    KLOG_DEBUG() << "addQuery cmd = " << cmd;
    if (!query.exec(cmd))
        KLOG_DEBUG() << "BoxDao insert error!";
}

void BoxDao::modifyQueryMountStatus(const QString boxId, bool isMount)
{
    QSqlQuery query(m_boxDb);

    QString cmd = QString("update notes set isMount ='%1' where boxId='%2';").arg(QString::number(isMount), boxId);
    KLOG_DEBUG() << "fixedQueryMountStatus cmd = " << cmd;
    if (!query.exec(cmd))
        KLOG_DEBUG() << "modifyQueryMountStatus error!";
}

void BoxDao::ModifyQueryPasswd(const QString boxId, const QString encryptpassword)
{
    QSqlQuery query(m_boxDb);

    QString cmd = QString("update notes set encryptpassword='%1' where boxId='%2';").arg(encryptpassword, boxId);
    KLOG_DEBUG() << "fixedQueryPasswd cmd = " << cmd;
    if (!query.exec(cmd))
        KLOG_DEBUG() << "ModifyQueryPasswd error!";
}

bool BoxDao::delQuery(const QString boxId)
{
    QSqlQuery query(m_boxDb);

    QString cmd = QString("delete from notes where boxId='%1';").arg(boxId);
    KLOG_DEBUG() << "addQuery cmd = " << cmd;
    if (!query.exec(cmd))
    {
        KLOG_DEBUG() << "BoxDao delete error! boxId = " << boxId;
        return false;
    }
    else
        return true;
}

QSqlQuery BoxDao::findQuery(const QString boxId)
{
    QSqlQuery query(m_boxDb);
    QString cmd = "select * from notes;";
    if (!query.exec(cmd))
        KLOG_DEBUG() << "select error!";
    else
    {
        while (query.next())
        {
            KLOG_DEBUG() << "boxName:" << query.value(0).toString();
            KLOG_DEBUG() << "boxId:" << query.value(1).toString();
            if (query.value(1).toString() == boxId)
                return query;
        }
    }
    return QSqlQuery();
}

int BoxDao::getQueryCount()
{
    QSqlQuery query(m_boxDb);
    QString cmd = "select count() from notes;";
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
    if (!query.exec("select count() from notes;"))
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
    m_boxDb.setDatabaseName(SC_INSTALL_DATADIR "/box-dao.dat");  //设置数据库名称
    if (!m_boxDb.open())
        KLOG_ERROR() << "BoxDao open error：" << m_boxDb.lastError();

    // 创建表
    QSqlQuery query(m_boxDb);

    //    if (!m_boxDb.isValid())
    if (!sqlTableExist(query))
    {
        query.exec("drop table notes");

        QString cmd = "create table notes(\
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
