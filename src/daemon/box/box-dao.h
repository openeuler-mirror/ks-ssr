#ifndef BOXDAO_H
#define BOXDAO_H

#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "lib/base/base.h"

namespace KS
{
class BoxDao
{
public:
    BoxDao();
    ~BoxDao();

    static BoxDao *getInstance()
    {
        return m_instance;
    };

    static QSqlDatabase getInstanceDb()
    {
        return m_instance->m_boxDb;
    };

    static void globalInit()
    {
        m_instance = new BoxDao();
        m_instance->init();
    };

    static void globalDeinit() { delete m_instance; };

    void addQuery(const QString boxName, const QString boxId, bool isMount, const QString encryptpassword, const QString encryptKey, const QString encryptPspr, int senderUserUid);
    void ModifyQueryMountStatus(const QString boxId, bool isMount);
    void ModifyQueryPasswd(const QString boxId, const QString encryptpassword);
    bool delQuery(const QString boxId);
    QSqlQuery findQuery(const QString boxId);
    int getQueryCount();

private:
    void init();

private:
    static BoxDao *m_instance;
    QSqlDatabase m_boxDb;

};
}  // namespace KS
#endif // BOXDAO_H
